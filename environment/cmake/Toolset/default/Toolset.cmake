
set(TOOLSET_VERSION 1)

find_program(GCC_COMMAND ${CMAKE_C_COMPILER})

# Get target architecture of the compiler
execute_process(
  COMMAND         ${GCC_COMMAND} -dumpmachine
  OUTPUT_VARIABLE TOOLSET_COMPILER_ARCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the version of the compiler
execute_process(
  COMMAND ${GCC_COMMAND} -dumpversion
  OUTPUT_VARIABLE TOOLSET_COMPILER_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Add the compile flags for the toolset
include(${CMAKE_CURRENT_LIST_DIR}/compile_flags.cmake)

# The other modules are called by their respective "main" module


