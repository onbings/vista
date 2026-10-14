import pathlib
import os
import sys

from environment import Environment
from process import process_cmd, process_cmd_silent, process_cmd_pipe_stdin

class Docker(object) :

    #
    # Description
    #   The class constructor
    #
    # Parameters
    #   _image_name    - The name of the image
    #   _image_version - The version of the image
    #   _remote_path   - The path to the remote registry
    #   _verbose       - The flag indicating if command should be verbose
    #
    # Returns
    #    Nothing
    #
    def __init__(self, _image_name, _image_version, _remote_path, _verbose = False) :
        self.image_name     = _image_name
        self.image_version  = _image_version
        self.remote_path    = _remote_path
        self.verbose        = _verbose
        self.parallel_build = os.cpu_count() - 1
        self.environ        = Environment()

    #
    # Description
    #   This function sets the number
    #   of compilation threads that can
    #   run in parallel
    #
    # Parameters
    #   _name - The number of threads
    #
    # Returns
    #    Nothing
    #
    def set_parallel_build(self, _value) :
        self.parallel_build = _value

    #
    # Description
    #   This function returns the number
    #   of compilation threads that can
    #   run in parallel
    #
    # Parameters
    #   None
    #
    # Returns
    #    The number of threads
    #
    def get_parallel_build(self) :
        return self.parallel_build

    #
    # Description
    #   This function sets the image name
    #
    # Parameters
    #   _name - The name of the image
    #
    # Returns
    #    Nothing
    #
    def set_image_name(self, _name) :
        self.image_name = _name

    #
    # Description
    #   This function sets the image version
    #
    # Parameters
    #   _version - The version of the image
    #
    # Returns
    #    Nothing
    #
    def set_image_version(self, _version) :
        self.image_version = _version

    #
    # Description
    #   This function returns the image name
    #
    # Parameters
    #   None
    #
    # Returns
    #   The image name
    #
    def get_image_name(self) :
        return self.image_name

    #
    # Description
    #   This function returns the image version
    #
    # Parameters
    #   None
    #
    # Returns
    #   The image version
    #
    def get_image_version(self) :
        return self.image_version

    #
    # Description
    #   This function returns the base path to
    #   the remote registry
    #
    # Parameters
    #   None
    #
    # Returns
    #   The registry path
    #
    def get_remote_path(self) :
        return self.remote_path

    #
    # Description
    #   This function returns name of
    #   the remote registry
    #
    # Parameters
    #   None
    #
    # Returns
    #   The name of the remote registry
    #
    def get_registry(self) :
        if '/' in self.remote_path :
            pos = self.remote_path.find('/')
            return self.remote_path[:pos]

        return self.remote_path

    #
    # Description
    #   This function returns the
    #   complete image name
    #
    # Parameters
    #   None
    #
    # Returns
    #   The complete image name (name:version)
    #
    def get_image(self) :
        return "{}:{}".format(self.get_image_name(), self.get_image_version())

    #
    # Description
    #   This function returns the
    #   complete image name from
    #   the remote registry
    #
    # Parameters
    #   None
    #
    # Returns
    #   The complete image name from the
    #   remote registry (registry/name:version)
    #
    def get_remote_image(self) :
        return "{}/{}".format(self.get_remote_path(), self.get_image())

    #
    # Description
    #   This function process the given command
    #   and display the result to the terminal
    #
    # Parameters
    #   _cmd - The command
    #
    # Returns
    #   Throws RuntimeError in case of failure
    #
    def process(self, _cmd) :
        if self.verbose :
            print("[verbose] >> {}".format(_cmd))

        current_dir = str(pathlib.Path(__file__).parent.absolute())
        docker_dir  = os.path.join(current_dir, "..", "dockers")

        status = process_cmd(_cmd, _cwd=docker_dir)

        if not status == 0 :
            raise RuntimeError("The command {} returned error {}".format(_cmd, status))

    #
    # Description
    #   This function process the given command
    #   and display the result to the terminal
    #
    # Parameters
    #   _cmd - The command
    #
    # Returns
    #   Throws RuntimeError in case of failure
    #
    def process_silent(self, _cmd) :
        if self.verbose :
            print("[verbose] >> {}".format(_cmd))

        current_dir = str(pathlib.Path(__file__).parent.absolute())
        docker_dir  = os.path.join(current_dir, "..", "dockers")

        status = process_cmd_silent(_cmd, _cwd=docker_dir)

        if not status == 0 :
            raise RuntimeError("The command {} returned error {}".format(_cmd, status))

    #
    # Description
    #   This function builds the given image
    #
    # Parameters
    #   _dockerfile        - The name of the Dockerfile
    #   _target            - The optional target in the Dockerfile
    #   _additional_params - Optional parameters to give to the docker build command
    #
    # Returns
    #   Throws RuntimeError in case of failure
    #
    def build(self, _dockerfile, _target = None, _additional_params = None) :

        cmd = ["docker", "build", "--force-rm", "-f", _dockerfile, "--network=host", "--build-arg", "PARALLEL_BUILD={}".format(self.parallel_build) ]

        if self.environ.is_linux() :
            cmd.extend(["--ulimit", "nofile={0}:{0}".format("1024000")])

        if self.environ.is_windows() :
            cmd.extend(["--memory", "8G"])

        if _target :
            cmd.extend([ "--target", _target ])

        # additional params if any
        if _additional_params :
            for arg in _additional_params :
                cmd.extend(["--build-arg", arg])

        cmd.extend([ "-t", self.get_image(), "." ])

        self.process(cmd)

    #
    # Description
    #   This function tags the image for the remote registry
    #
    # Parameters
    #   _src - The image to tag
    #   _dst - The new tag name
    #
    # Returns
    #   Throws RuntimeError in case of failure
    #
    def tag(self, _src, _dst) :

        cmd = [ "docker", "tag", _src, _dst ]

        self.process(cmd)

    #
    # Description
    #   This function pushes the image to the remote registry
    #
    # Parameters
    #   _image - The image to push
    #
    # Returns
    #   Throws RuntimeError in case of failure
    #
    def push(self, _image) :

        cmd = [ "docker", "push", _image ]

        self.process(cmd)

    #
    # Description
    #   This function removes the image from our local drive
    #
    # Parameters
    #   _image - The image
    #
    # Returns
    #   Throws RuntimeError in case of failure
    #
    def remove_image(self, _image) :

        cmd = [ "docker", "image", "remove", "--force", _image ]

        self.process(cmd)

    #
    # Description
    #   This function removes both the built and registry images
    #
    # Parameters
    #   None
    #
    # Returns
    #   Throws RuntimeError in case of failure
    #
    def remove(self) :

        self.remove_image(self.get_image())
        self.remove_image(self.get_remote_image())

    #
    # Description
    #   This function tags and publishes the built
    #   image to the remote registry
    #
    # Parameters
    #   None
    #
    # Returns
    #   Throws RuntimeError in case of failure
    #
    def publish(self) :

        self.tag (self.get_image(), self.get_remote_image())
        self.push(self.get_remote_image())

    #
    # Description
    #   This function logs in to the remote registry
    #
    # Parameters
    #   user     - The user name
    #   password - The user password
    #
    # Returns
    #   Throws RuntimeError in case of failure
    #
    def login(self, user, password) :

        cmd = [ "docker", "login", "--username", user, "--password", password, self.get_registry() ]

        self.process_silent(cmd)

    #
    # Description
    #   This function logs out from the remote registry
    #
    # Parameters
    #   None
    #
    # Returns
    #   Throws RuntimeError in case of failure
    #
    def logout(self) :

        cmd = [ "docker", "logout", self.get_registry() ]

        self.process(cmd)

    #
    # Description
    #   This function indicates whether the image
    #   already exists locally or not
    #
    # Parameters
    #   None
    #
    # Returns
    #   True  - The image exists locally
    #   False - The image does not exist locally
    #
    def exist_locally(self) :

        cmd = [ "docker", "image", "inspect", self.get_remote_image() ]

        return process_cmd_silent(cmd) == 0

    #
    # Description
    #   This function indicates whether the image
    #   already exists remotely or not
    #
    # Parameters
    #   None
    #
    # Returns
    #   True  - The image exists remotely
    #   False - The image does not exist remotely
    #
    def exist_remotely(self) :

        cmd = [ "docker", "manifest", "inspect", self.get_remote_image() ]

        return process_cmd_silent(cmd) == 0
