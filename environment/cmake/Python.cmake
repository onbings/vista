# Make sure to find the Python interpreter on the host system
# and not some potential candidate in the sysroot if any
find_program(
  HOST_PYTHON_EXECUTABLE
  NAMES python3 python
  NO_CMAKE_FIND_ROOT_PATH
  NO_CACHE
  REQUIRED
)

set(Python_EXECUTABLE  ${HOST_PYTHON_EXECUTABLE})
set(Python3_EXECUTABLE ${HOST_PYTHON_EXECUTABLE})
unset(HOST_PYTHON_EXECUTABLE)
find_package(Python ${SEARCH_PYTHON_VERSION} COMPONENTS Interpreter REQUIRED)

message(STATUS "Python::Interpreter is at ${Python_EXECUTABLE}")