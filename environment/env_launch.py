#!/usr/bin/env python3

import argparse
from collections import OrderedDict
import datetime
import glob
import json
import os
import pathlib
import shutil
import subprocess
import sys
import time

# Register modules in automation
sys.path.insert(1, os.path.join(str(pathlib.Path(__file__).parent.resolve()), "automation"))

# Load the image catalog
from docker  import Docker
from environment import Environment
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
    environment        = Environment()

    # =========================
    # == Parse the arguments ==
    # =========================

    parser = argparse.ArgumentParser()

    parser._action_groups.pop()
    required = parser.add_argument_group('Required arguments')
    optional = parser.add_argument_group('Optional arguments')

    # Mandatory arguments
    required.add_argument("-n", "--name", dest="name",     help = "The name of the environment to launch (found : {})".format(known_environments), required=True)

    # Optional arguments
    optional.add_argument("--build",      dest="build",    help = "This flag forces building from Dockerfile", action='store_true')
    optional.add_argument("--user",       dest="user",     help = "The docker user name to publish artifacts")
    optional.add_argument("--password",   dest="password", help = "The docker user password to publish artifacts")

    optional = docker_run.add_optional_arguments(optional)

    args = parser.parse_args(argv)

    # Create args to forward
    forward_args = erase_value_option (sys.argv[1:], ["--user", "--password"])
    forward_args = erase_single_option(forward_args, ["--build"])

    # =======================
    # == Get the arguments ==
    # =======================

    name     = args.name.lower()
    user     = args.user
    password = args.password
    build    = args.build
    verbose  = args.verbose

    hasher     = PathHash("__*")
    hash       = hasher.get_hash_of_path(os.path.join(current_dir, "dockers"))
    image_name = "{}:{}".format(name, hash)

    # Get path to Artifactory
    with open(os.path.join(current_dir, "artifactory.repo"), "r") as file :
        artifactory_repo = file.read().strip()

    docker         = Docker(name, hash, artifactory_repo, verbose)
    dockerfile     = os.path.join(current_dir, "dockers", "Dockerfile.{}".format(name))
    has_dockerfile = os.path.isfile(dockerfile)

    print("Selected environment         : {}".format(name))
    print("Hash                         : {}".format(hash))
    print("Associated dockerfile        : {}".format(dockerfile))
    print("Dockerfile exists            : {}".format(has_dockerfile))
    print("Docker image exists locally  : {}".format(docker.exist_locally()))
    print("Docker image exists remotely : {}".format(docker.exist_remotely()))

    if not build and (docker.exist_locally() or docker.exist_remotely()) :
        forward_args = substitute_option(forward_args, ["-n", "--name"], "-i", docker.get_remote_image())
        return docker_run.main(forward_args)

    if not has_dockerfile :
        print("No image exist for {} and no Dockerfile exist to build environment {}".format(docker.get_image(), name))
        return -1

    # Build Dockerfile
    docker.build(dockerfile, "image_devel")

    # Publish it
    if user and password :
        try :
            docker.login(user, password)
            docker.publish()
        finally :
            docker.logout()

    # Run it
    forward_args = substitute_option(forward_args, ["-n", "--name"], "-i", docker.get_image())

    return docker_run.main(forward_args)

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
