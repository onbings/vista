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
import tempfile

# Register modules in automation
sys.path.insert(1, os.path.join("."))

# Load our own modules
from environment import Environment
from process import process_cmd
from passwd import Passwd
from group import Group

#
# Description
#   This function parses the volumes options
#   that are given as <host path>:<docker path>
#
# Parameters
#   _mapping - The mapping
#
# Returns
#   The list of extracted tokens
#
# Remarks
#  Windows path can contain : in the path
#
def ExtractVolumes(_mapping) :

    found = False
    idx = 0

    while not found :

        # We must find the first : that is not :\
        idx = _mapping.find(":", idx)

        if idx == -1 :
            return []

        if not _mapping[idx + 1] == '\\' :
            return [ _mapping[:idx], _mapping[idx + 1:] ]

        idx = idx + 2

    return []

#
# Description
#   This function lists the docker volumes
#
# Parameters
#   None
#
# Returns
#   The list of Docker volumes
#
# Remarks
#  None
#
def list_docker_volumes() :
    ret = []
    response = subprocess.check_output(['docker', 'volume', 'ls']).decode('utf-8')

    idx = 0
    for line in response.split('\n') :
        if idx > 0 :
            tokens = line.split()
            if len(tokens) == 2 :
                ret.append(tokens[1])

        idx += 1

    return ret

#
# Description
#   This function add the mount volume
#   to the command and ensures that the
#   host part of the volume is created
#   before entering the docker
#
# Parameters
#   _cmd        - The command
#   _volume     - The volume
#
# Returns
#   True  - The volume was added to the command
#   False - The volume was not added to the command
#
# Remarks
#  None
#
def mount_volume(_cmd, _volume) :

    # volume should be defined as host:docker
    tokens=ExtractVolumes(_volume)

    if len(tokens) == 2 :
        host=os.path.abspath(tokens[0])
        dest=tokens[1]

        # Is it a known path ?
        path = pathlib.Path(host)

        if not path.exists() :

            # This is a Docker volume
            if tokens[0] in list_docker_volumes() :
                host = tokens[0]
            # Assume it's a non-existing directory
            else :
                 # Make sure the host past exists before entering
                 # the docker to have user permissions rather than
                 # root.
                path.mkdir(parents=True, exist_ok=True)

        _cmd.extend(["-v", "{}:{}".format(host, dest)])

        return True
    else :
        print("Invalid -v {}".format(vol))
        return False

#
# Description
#   This function checks if the given
#   volume is already mounted on the
#   destination (container side)
#
# Parameters
#   _cmd         - The current command line
#   _dest_volume - The mount point on the destination
#
# Returns
#   True  - The volume is already mounted
#   False - The volume is not yet mounted
#
# Remarks
#  None
#
def is_volume_already_mounted(_cmd, _dest_volume) :

    is_volume_mapping = False

    # Check each arguments of command
    for cmd_item in _cmd :

        if is_volume_mapping :

            # Extract mapping 'source:dest{:access}'
            tokens = ExtractVolumes(cmd_item)

            if len(tokens) >= 2 :
                host=os.path.abspath(tokens[0])
                dest=tokens[1]

                # Does the destination matches ?
                if dest.startswith(_dest_volume) :
                    return True

            is_volume_mapping = False

        # Next argument will be a mapping
        if cmd_item == '-v' :
            is_volume_mapping = True

    return False

#
# Description
#   This function adds the optional arguments
#   to the parser
#
# Parameters
#   optional - The optional argument parser
#
# Returns
#   The optional argument parser
#
def add_optional_arguments(optional) :
    optional.add_argument("-c", "--cmd",            dest="cmd",            help = "An optional command to run inside the docker container")
    optional.add_argument("-w", "--working_dir",    dest="working_dir",    help = "An optional directory to start in the container")
    optional.add_argument("-v", "--volume",         dest="volume",         help = "An optional list <local>:<container> of path to bind inside the container", action='append', nargs='*')
    optional.add_argument("-p", "--port",           dest="port",           help = "An optional list <local>:<container> of ports to map", action='append', nargs='*')
    optional.add_argument("-e", "--env",            dest="env",            help = "An optional list <var>=<value> of environment variables", action='append', nargs='*')
    optional.add_argument(      "--read-only",      dest="read_only",      help = "The flag indicating all volume mounted should be read-only", action='store_true')
    optional.add_argument(      "--verbose",        dest="verbose",        help = "Print verbose information about the underlying command", action='store_true')
    optional.add_argument(      "--version",        dest="version",        help = "An optional version to override the current one")
    optional.add_argument(      "--memory",         dest="memory",         help = "An optional memory parameter to update available RAM for Windows containers")
    optional.add_argument(      "--gpu",            dest="gpu",            help = "Mount GPU into the container", action='store_true')
    optional.add_argument(      "--network",        dest="network",        help = "Mount the host network inside the container", action='store_true')
    optional.add_argument(      "--container-name", dest="container_name", help = "The name to give to the container")

    return optional

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

    current_dir = str(pathlib.Path(__file__).parent.absolute())
    environment = Environment()

    # =========================
    # == Parse the arguments ==
    # =========================

    parser = argparse.ArgumentParser()

    parser._action_groups.pop()
    required = parser.add_argument_group('Required arguments')
    optional = parser.add_argument_group('Optional arguments')

    # Mandatory arguments
    required.add_argument("-i", "--image", dest="image", help = "The image name", required=True)

    # Optional arguments
    optional = add_optional_arguments(optional)

    args = parser.parse_args(argv)

    # =======================
    # == Get the arguments ==
    # =======================

    image = args.image.lower()

    if args.read_only :
        volume_mode = "ro"
    else :
        if environment.is_linux() :
            volume_mode = "delegated"
        else :
            volume_mode = "rw"

    is_interactive = not args.cmd

    # ==============================
    # == Build the docker command ==
    # ==============================

    # Create a temporary directory to store files
    temp_dir_obj = tempfile.TemporaryDirectory()
    temp_dir     = temp_dir_obj.name

    cmd = [ "docker", "run", "--rm", ]

    # Is it interactive
    if is_interactive :
        cmd.append("-it")

    # Bind mandatory environment variables
    cmd.extend(["-e", "CMAKE_BUILD_PARALLEL_LEVEL={}".format(os.cpu_count() - 1)])

    # Forward variables for Teamcity
    for variable in environment.environ :
        if variable.startswith("TEAMCITY") :
            cmd.extend(["-e", "{}={}".format(variable, environment.environ[variable])])
        if variable.startswith("VCPKG_") :
            cmd.extend(["-e", "{}={}".format(variable, environment.environ[variable])])
        if variable.startswith("JFROG_CLI_") :
            cmd.extend(["-e", "{}={}".format(variable, environment.environ[variable])])
        if variable.startswith("SSH_AUTH_SOCK") :
            sock = environment.environ[variable]
            path = os.path.dirname(sock)

            # Make sure path is not empty
            if path :
                cmd.extend(["-e", "{}={}".format(variable, sock)])
                cmd.extend(["-v", "{}:{}:ro".format(path, path)])
                print("Forwarding {} sock {} path {}".format(variable, sock, path))

    # Bind user-defined volumes
    if args.volume :
        for volume in args.volume :
            for vol in volume :
                if not mount_volume(cmd, "{}:{}".format(vol, volume_mode)) :
                    return 1

    # Bind user-defined ports
    if args.port :
        for port in args.port :
            for p in port :
                cmd.extend(["-p", p])

    # Bind user-defined environment variables
    if args.env :
        for env in args.env :
            for e in env :
                cmd.extend(["-e", e])

    user = environment.GetUser()
    home_user = environment.GetHomeDir()

    # Linux specific
    if environment.is_linux() :

        cmd.append("--privileged")

        # Enforce ulimit
        cmd.extend(["--ulimit", "nofile={0}:{0}".format("1024000")])

        # Add user id and group id
        from pwd import getpwnam
        user_access = getpwnam(user)

        uid         = user_access.pw_uid
        gid         = user_access.pw_gid
        passwd_path = "/etc/passwd"
        group_path  = "/etc/group"

        passwd = Passwd(passwd_path)
        group  = Group (group_path)

        # Does our user exist in passwd ?
        if not passwd.is_user_in(user) :

            # Create a temporary named file
            passwd_path = os.path.join(temp_dir, "passwd")

            # Write our user into it
            passwd.add_entry(user, uid, gid)
            passwd.write_to (passwd_path)

        # Does our group exist in group ?
        if not group.is_group_id_in(gid) :

            # Create a temporary named file
            group_path = os.path.join(temp_dir, "group")

            # Write our group into it
            group.add_entry(gid, user)
            group.write_to (group_path)

        # Specify the user IDs
        cmd.extend(["-u", "{}:{}".format(uid, gid)])
        cmd.extend(["-e", "USER={}".format(user)])
        cmd.extend(["-e", "HOME={}".format(home_user)])

        # Mount volumes
        mount_volume(cmd, "{}:/etc/group:ro".format(group_path))
        mount_volume(cmd, "{}:/etc/passwd:ro".format(passwd_path))
        mount_volume(cmd, "/etc/shadow:/etc/shadow:ro")

        # Vulkan
        if os.path.isdir("/etc/vulkan") :
            mount_volume(cmd, "/etc/vulkan:/etc/vulkan:ro")

        if os.path.isdir("/usr/share/vulkan") :
            mount_volume(cmd, "/usr/share/vulkan:/usr/share/vulkan:ro")

        if os.path.isdir("/usr/share/glvnd") :
            mount_volume(cmd, "/usr/share/glvnd:/usr/share/glvnd:ro")

        # Use user's cache directory for better performance and persistence
        nuget_base = os.path.join(home_user, ".nuget")

        # If NuGet cache does not exist, create it
        if not os.path.isdir(nuget_base) :
            os.makedirs(nuget_base, exist_ok=True)

        # This is an interactive shell
        if is_interactive :
            mount_volume(cmd, "{0}:{0}:{1}".format(home_user, volume_mode))
        else :

            # Mount all the configuration folders (e.g. /home/user/.*)
            if not is_volume_already_mounted(cmd, home_user) :

                config_dirs = [ f.name for f in os.scandir(home_user) if (f.is_dir() and f.name.startswith('.'))]

                for dir in config_dirs :

                    # By default they are read-only
                    volume_property = ":ro"

                    # Lift restriction for specific dirs
                    if dir in [ ".cache", ".config", ".jfrog", ".nuget" ] :
                        volume_property = ""

                    mount_volume(cmd, "{0}/{1}:{0}/{1}{2}".format(home_user, dir, volume_property))

    # Windows specific
    else :

        container_user = "ContainerUser"

        cmd.extend(["-e", "DOCKER_USER={}".format(user)])
        #cmd.extend(["-u", "{}".format('ContainerUser')])

        # Setup resources (required due to Hyper-V limitations under Windows
        # which limits CPUs to 2 and memory to 1GB by default when using the Windows
        # backend)
        cmd.extend(["--cpu-count", "{}".format(os.cpu_count() - 1)])

        if args.memory :
            memory = args.memory
        else :
            memory = "8GB"

        cmd.extend(["--memory", memory])

        # Work-around NuGet cache which might exceed path limitation in container
        nuget_base = os.path.join("C:\\", "nuget")

        # Make ssh work in the container
        if os.path.isdir(os.path.join(home_user, ".ssh")) :

            # Load ssh keys to user directory as read-only (they will be copied by entrypoint script)
            mount_volume(cmd, r"{0}\.ssh:{0}\.ssh:ro".format(home_user))

        # Mount all the configuration folders (e.g. %userprofile%.*)
        config_dirs = [ f.name for f in os.scandir(home_user) if (f.is_dir() and f.name.startswith('.'))]

        for dir in config_dirs :

            # Skip .ssh as it will be copied by entrypoint script
            # Rationale : We cannot match user on the host with
            # the user in the container. As such, ssh key if loaded
            # from the host user will not belong to container user and
            # thus will be rejected (bad permissions). Thus the script
            # is copying them at startup to have the right ownership
            if ".ssh" in dir :
                continue

            # By default they are read-only
            volume_property = ":ro"

            # Lift restriction for specific dirs
            if dir in [ ".nuget", ".config", ".jfrog" ] :
                volume_property = ""

            host_dir = r"{0}\{1}".format(home_user, dir)
            dest_dir = r"C:\Users\{0}\{1}".format(container_user, dir)

            if not is_volume_already_mounted(cmd, dest_dir) :
                mount_volume(cmd, r"{0}:{1}{2}".format(host_dir, dest_dir, volume_property))

        # Do not override if home user is already mounted
        if not is_volume_already_mounted(cmd, home_user) :

            # Check if we need to mount user vcpkg cache
            binary_cache_paths = OrderedDict([
              ("VCPKG_DEFAULT_BINARY_CACHE", ""),
              ("LOCALAPPDATA"              , "vcpkg"),
              ("APPDATA"                   , "vcpkg"),
              ("XDG_CACHE_HOME"            , "vcpkg"),
              ("HOME"                      , os.path.join(".cache", "vcpkg"))
            ])

            for var,subdir in binary_cache_paths.items() :
                if var in environment.environ :
                    dir_path = os.path.join(environment.environ[var], subdir)

                    # We found the host vcpkg cache. Load it to the container
                    if os.path.isdir(dir_path) :
                        container_path = r"{0}\AppData\Local\vcpkg".format(home_user)
                        mount_volume(cmd, r"{0}:{1}".format(dir_path, container_path))
                        cmd.extend(["-e", "VCPKG_DEFAULT_BINARY_CACHE={0}".format(container_path)])
                        break

    cmd.extend(["-e", "NUGET_PACKAGES={}".format(os.path.join(nuget_base, "packages"))])
    cmd.extend(["-e", "NUGET_HTTP_CACHE_PATH={}".format(os.path.join(nuget_base, "cache"))])
    cmd.extend(["-e", "NUGET_PLUGINS_CACHE_PATH={}".format(os.path.join(nuget_base, "plugins"))])

    # Mount GPUs ?
    if args.gpu :
        cmd.extend(["--gpus", "all"])

    # Mount network ?
    if args.network :
        cmd.extend(["--network=host"])

    # Set the name ?
    if args.container_name :
        cmd.extend(["--name", args.container_name ])

    # Set the working dir
    if args.working_dir :
        working_dir = args.working_dir
    else :
        if environment.is_linux() :
            working_dir = "/home/{}".format(user)
        else :
            working_dir = "C:\\"

    cmd.extend(["-w", working_dir])

    # Add the required image
    cmd.append(image)

    # Add command if non-interactive
    if args.cmd :
        if environment.is_linux() :
            cmd.append("bash")
            cmd.append("-c")
            cmd.append(args.cmd)
        else :
            cmd.append("cmd")
            cmd.append("/c")
            cmd.append(args.cmd)

    # =================================
    # == Start command inside docker ==
    # =================================

    print("Starting docker for image {}".format(image))

    # Debug the command
    if args.verbose :
        print("Cmd is {}".format(cmd))

    start = datetime.datetime.now()

    # Docker for Windows often fails with error 125
    # meaning "Ther requested resource is in use".
    # Simply retrying solve the issue
    for i in range(100) :

        # Execute the command
        status = process_cmd(cmd)

        if status != 125 or not environment.is_windows() :
            break
        else :
            print("Detected resource in use error. Retrying in 1s")
            time.sleep(1)

    end = datetime.datetime.now()

    # Delete temporary dir
    shutil.rmtree(temp_dir)

    print("Process is done with error code {} in {} seconds".format(status, (end - start).total_seconds()))

    return status

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
