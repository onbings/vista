find_program(IWYU include-what-you-use)

if(NOT "${IWYU}" STREQUAL "IWYU-NOTFOUND")

  # Get the version
  execute_process(
    COMMAND ${IWYU} --version
    OUTPUT_VARIABLE IWYU_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Tell cmake to use it
  set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU})

endif()
