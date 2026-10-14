find_program(CLANGTIDY clang-tidy)

if(NOT "${CLANGTIDY}" STREQUAL "CLANGTIDY-NOTFOUND")

  # Get the version
  execute_process(
    COMMAND ${CLANGTIDY} --version
    OUTPUT_VARIABLE  CLANGTIDY_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  string(REGEX MATCH "LLVM version [0-9\\.]*" ver_line "${CLANGTIDY_VERSION}")
  string(REPLACE "LLVM version" "" CLANGTIDY_VERSION "${ver_line}")
  string(STRIP "${CLANGTIDY_VERSION}" CLANGTIDY_VERSION)

  # Tell CMake to use it
  set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY})

endif ()

