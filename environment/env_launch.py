#!/usr/bin/env python3

import argparse
import os
import pathlib
import sys

# Register modules in automation
sys.path.insert(1, os.path.join(str(pathlib.Path(__file__).parent.resolve()), "automation"))

# Load the image catalog
from docker  import Docker
from path_hash import PathHash
import docker_run

#
# Description
#   This function lists the "Dockerfile.x" in the
#   given path and extract the associated platform
#
# Parameters
#   path - The path to scan
#
# Returns
#   The list of supported environment
#
def list_supported_environment(path) :

    images = []

    for dirpath, dirname, files in os.walk(path) :
        for file in files :
            if file.startswith("Dockerfile.") :
                pos = file.find('.') + 1
                images.append(file[pos:])

    return images

#
# Description
#   This function takes a string of arguments
#   and substitute options (and associated value)
#   by a new option and a new value
#
# Parameters
#   args       - The full argument string
#   options    - The options to match
#   new_option - The new replacement option name
#   value      - The replacement value
#
# Returns
#   The list of supported environment
#
def substitute_option(args, options, new_option, value) :

    for i in range(0, len(args)) :
        arg = args[i]

        if arg in options :
            args[i + 0] = new_option
            args[i + 1] = value

    return args

#
# Description
#   This function takes a string of arguments
#   and removes options and associated value
#
# Parameters
#   args       - The full argument string
#   options    - The options to match
#
# Returns
#   The new arguments
#
def erase_value_option(args, options) :

    ret = []
    skip = False

    for i in range(0, len(args)) :
        arg = args[i]

        # Found skip option and value
        if arg in options :
            skip = True
        else :
            add = not skip
            skip = False

            if add :
                ret.append(arg)

    return ret

#
# Description
#   This function takes a string of arguments
#   and removes options and associated value
#
# Parameters
#   args       - The full argument string
#   options    - The options to match
#
# Returns
#   The new arguments
#
def erase_single_option(args, options) :

    ret = []

    for i in range(0, len(args)) :
        arg = args[i]

        # Found skip option and value
        if not arg in options :
            ret.append(arg)

    return ret

def ensure_file_exists(path):
    if not os.path.exists(path):
        raise FileNotFoundError("File does not exist: {}".format(path))

def write_devcontainer_yml(path, image):
    # Make sure the directory exists
    os.makedirs(os.path.dirname(path), exist_ok=True)
    
    with open(path, "w") as f:
        f.write("services:\n")
        f.write("  dev:\n")
        f.write("    image: {}\n".format(image))

class ImageTarget(object):
    def __init__(
            self,
            name,
            tag,
            remote_repo,
            dockerfile=None,
            target=None,
            additional_params=None,
            force_build=False,
            user=None,
            password=None,
            verbose=False,
        ) -> None:
        self.docker = Docker(name, tag, remote_repo, dockerfile, verbose)

        self.target = target
        self.additional_params = additional_params
        self.force_build = force_build
        self.user = user
        self.password = password

    def print(self):
        print("Docker image name            : {}".format(self.docker.get_image_name()))
        print("Docker image version         : {}".format(self.docker.get_image_version()))
        print("Associated dockerfile        : {}".format(self.docker.get_dockerfile()))
        print("Dockerfile exists            : {}".format(self.docker.dockerfile_exists()))
        print("Docker image exists locally  : {}".format(self.docker.exist_locally()))
        print("Docker image exists remotely : {}".format(self.docker.exist_remotely()))
        
    def ensure_exists_locally(self):
        # Nothing to do
        if self.docker.exist_locally() and not self.force_build:
            return

        # Pull from remote
        if self.docker.exist_remotely() and not self.force_build:
            self.docker.pull(self.docker.get_remote_image())
            self.docker.tag (self.docker.get_remote_image(), self.docker.get_image())
            return

        # Rebuild
        ensure_file_exists (self.docker.get_dockerfile())
        self.docker.build  (self.docker.get_dockerfile(), self.target, self.additional_params)
        self.docker.publish(self.user, self.password)
            

#
# Description
#   This is the entry point of the script
#
# Parameters
#   Dir - The base directory to scan
#
# Returns
#    0 - The operation was successful
#   !0 - The operation failed
#
def main(argv):

    current_dir        = str(pathlib.Path(__file__).parent.absolute())
    known_environments = list_supported_environment(os.path.join(current_dir, "dockers"))

    # =========================
    # == Parse the arguments ==
    # =========================

    parser = argparse.ArgumentParser()

    parser._action_groups.pop()
    required = parser.add_argument_group('Required arguments')
    optional = parser.add_argument_group('Optional arguments')

    # Mandatory arguments
    required.add_argument("-n", "--name",        dest="name",              help = "The name of the environment to launch (found : {})".format(known_environments), required=True)

    # Optional arguments
    optional.add_argument("--build",             dest="build",             help = "This flag forces building from Dockerfile", action='store_true')
    optional.add_argument("--user",              dest="user",              help = "The docker user name to publish artifacts")
    optional.add_argument("--password",          dest="password",          help = "The docker user password to publish artifacts")
    optional.add_argument("--extensions",        dest="extensions",        help = "The comma separated paths to the extension Dockerfiles", )
    optional.add_argument("--image-name",        dest="image_name",        help = "The name of the docker image to use")
    optional.add_argument("--devcontainer",      dest="devcontainer",      help = "The path to the dev container configuration file")

    optional = docker_run.add_optional_arguments(optional)

    args = parser.parse_args(argv)

    # Create args to forward
    forward_args = erase_value_option (sys.argv[1:], ["--user", "--password", "--extensions", "--image-name", "--devcontainer"])
    forward_args = erase_single_option(forward_args, ["--build"])

    # =======================
    # == Get the arguments ==
    # =======================

    name         = args.name.lower()
    user         = args.user
    password     = args.password
    build        = args.build
    verbose      = args.verbose
    devcontainer = args.devcontainer

    hasher       = PathHash("__*")
    hash         = hasher.get_hash_of_path(os.path.join(current_dir, "dockers"))

    # Get path to Artifactory
    with open(os.path.join(current_dir, "artifactory.repo"), "r") as file :
        artifactory_repo = file.read().strip()

    docker = Docker(name, hash, artifactory_repo, os.path.join(current_dir, "dockers", "Dockerfile.{}".format(name)), verbose)

    images = [ 
        # The base image
        ImageTarget(
            name,
            hash,
            artifactory_repo,
            os.path.join(current_dir, "dockers", "Dockerfile.{}".format(name)),
            "image_devel",
            None,
            build,
            user,
            password,
            verbose
        ) 
    ]

    def chain_image(name, version, dockerfile, build, publish):
        last_image = images[-1].docker.get_image()

        images.append(
            ImageTarget(
                f"{name}-ext{len(images)}",
                version,
                artifactory_repo,
                dockerfile,
                None,
                [ f"BASE_IMAGE={last_image}" ],
                build,
                user if publish else None,
                password if publish else None,
                verbose
            )
        )
    
    if args.extensions:
        for extension in args.extensions.split(","):
            extension_path = pathlib.Path(extension).resolve().absolute()
            if not extension_path.exists():
                print(f"Extension path does not exist: {extension_path}. Skipping it.")
                continue
            extension_hash = hasher.get_hash_of_path(extension_path.parent)

            chain_image(name, extension_hash, str(extension_path), build, publish=True)

    if devcontainer:
        chain_image(name, hash, os.path.join(current_dir, "dockers", "Dockerfile.devcontainer"), build=True, publish=False)
        write_devcontainer_yml(devcontainer, images[-1].docker.get_image())

    image = images[0].docker.get_image()
    
    for target in images:
        target.print()
        target.ensure_exists_locally()
        
        image = target.docker.get_image()

    if args.image_name :
        docker.tag(image, args.image_name)

    return docker_run.main(substitute_option(forward_args, ["-n", "--name"], "-i", image))


#
# Description
#   The script main wrapper
#
if __name__ == "__main__":
    try :
        sys.exit(main(sys.argv[1:]))
    except Exception as e :
        print(e)
        sys.exit(1)
