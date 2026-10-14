import os
import sys

#
# Description
#  This function indicates if we are under linux
#
# Parameters
#  None
#
# Returns
#  True  - The system is linux
#  False - The system is not linux
#
def is_linux() :
    return sys.platform == "linux"

#
# Description
#  This function indicates if we are under windows
#
# Parameters
#  None
#
# Returns
#  True  - The system is windows
#  False - The system is not windows
#
def is_windows() :
    return sys.platform == "win32"

class Environment(object):

    #
    # Description
    #  The class constructor
    #
    # Parameters
    #  _Env - The environment object
    #
    # Returns
    #  The property value
    #
    def __init__(self, _Env = os.environ) :
        self.environ = _Env

    #
    # Description
    #  This function indicates if we are under linux
    #
    # Parameters
    #  None
    #
    # Returns
    #  True  - The system is linux
    #  False - The system is not linux
    #
    def is_linux(self) :
        return is_linux()

    #
    # Description
    #  This function indicates if we are under windows
    #
    # Parameters
    #  None
    #
    # Returns
    #  True  - The system is windows
    #  False - The system is not windows
    #
    def is_windows(self) :
        return is_windows()
    
    #
    # Description
    #  This function checks if the given
    #  environment variable is set and
    #  returns it's value if this is the
    #  case. Otherwise, it returns the
    #  default value
    #
    # Parameters
    #  _Variable - The name of the variable
    #  _Default  - The default value
    #
    # Returns
    #  The property value
    #
    def GetOrDefault(self, _Variable, _Default) :
        if _Variable in self.environ :
            return self.environ[_Variable]
        else :
            return _Default

    #
    # Description
    #  This function returns the path
    #  to the home directory of the
    #  currently logged user
    #
    # Parameters
    #  None
    #
    # Returns
    #  The path to the home directory
    #
    def GetHomeDir(self) :
        if self.is_linux() :
            return self.environ["HOME"]
        else :
            return self.environ["USERPROFILE"]

    #
    # Description
    #  This function returns the path
    #  to the home directory of the
    #  currently logged user
    #
    # Parameters
    #  None
    #
    # Returns
    #  The path to the home directory
    #
    def GetUser(self) :
        if self.is_linux() :
            return self.environ["USER"]
        else :
            return self.environ["USERNAME"]

    #
    # Description
    #  This function sets the environment variable
    #  to the given environment if it does not
    #  not exist yet
    #
    # Parameters
    #  _Variable - The name of the variable
    #  _Value    - The value
    #
    # Returns
    #  The property value
    #
    def SetIfNotExisting(self, _Variable, _Value) :
        if not _Variable in self.environ :
            self.environ[_Variable] = _Value

    #
    # Description
    #   This function check that the given environment
    #   variable is defined. If not, it displays an error
    #   message and exists the process
    #
    # Parameters
    #   _Variable - The name of the variable
    #
    # Returns
    #   Nothing
    #
    def AssertExists(self, _Variable) :
        if not _Variable in self.environ :
            print("The environment variable {} is not defined and no default value can be inferred. Please set it accordingly".format(_Variable))
            sys.exit(1)
