#=======================================================================
#
# This file is intended to add everything related to CMake/CTest testing
#
#=======================================================================

# Pre-stage binaries like if they were already installed
#  - CMAKE_LIBRARY_OUTPUT_DIRECTORY is for shared libraries
#  - CMAKE_ARCHIVE_OUTPUT_DIRECTORY is for static libraries
#  - CMAKE_RUNTIME_OUTPUT_DIRECTORY is for applications
set(COMPILED_BINARIES_DIR ${CMAKE_BINARY_DIR}/binaries)

if(WIN32)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${COMPILED_BINARIES_DIR}/bin)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${COMPILED_BINARIES_DIR}/bin)
else()
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${COMPILED_BINARIES_DIR}/lib)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${COMPILED_BINARIES_DIR}/bin)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${COMPILED_BINARIES_DIR}/lib)

# For Valgrind check
include(${CMAKE_CURRENT_LIST_DIR}/Testing/MemoryChecker.cmake)

# For code coverage
include(${CMAKE_CURRENT_LIST_DIR}/Testing/Coverage.cmake)

# Populate test
include(CTest)
