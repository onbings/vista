#=======================================================================
#
# This file is intended to add everything related to Memory check
#
#=======================================================================

if(TARGET report_valgrind)
  message(STATUS "report_valgrind already exists. Skipping")
  return()
endif()

# Is valgrind available ?
find_program(MEMORYCHECK_COMMAND valgrind)

# Valgrind is available
if(NOT "${MEMORYCHECK_COMMAND}" STREQUAL "MEMORYCHECK_COMMAND-NOTFOUND")

  # Get the version
  execute_process(
    COMMAND ${MEMORYCHECK_COMMAND} --version
    OUTPUT_VARIABLE  MEMORYCHECK_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  set(MEMORYCHECK_COMMAND_OPTIONS   "-q --tool=memcheck --leak-check=full --track-origins=yes -v --gen-suppressions=all --num-callers=50 --error-exitcode=1" CACHE STRING "The options to pass to valgrind" FORCE)
  set(MEMORYCHECK_SUPPRESSIONS_FILE "${CMAKE_BINARY_DIR}/valgrind_suppression.txt" CACHE PATH "The path to the suppression file" FORCE)

  # Populate with default suppressions
  file(READ  ${CMAKE_CURRENT_LIST_DIR}/valgrind_default_suppressions.txt FILE_CONTENT)
  file(WRITE ${MEMORYCHECK_SUPPRESSIONS_FILE} ${FILE_CONTENT})

else()

  # In that case, don't fail the test, but rather run it
  # without memory checking so that automated tests
  # can still try to run the "ctest -T Memcheck" without
  # failing if memory checking is not supported.
  find_program(MEMORYCHECK_COMMAND bash)

  set(MEMORYCHECK_TYPE "UndefinedBehaviorSanitizer")
  set(MEMORYCHECK_COMMAND_OPTIONS "-c" CACHE STRING "Memory checking is not supported" FORCE)

endif()

# Create file to report coverage information to Teamcity
file(WRITE ${CMAKE_BINARY_DIR}/report_valgrind_teamcity.sh
  "#!/bin/bash\n"
  "\n"
  "# Find the file containing the failed tests\n"
  "FILE=$(find ./Testing -type f -name LastTestsFailed_*.log)\n"
  "\n"
  "if [ -z \"$FILE\" ]; then\n"
  "  echo \"No failed tests found\"\n"
  "  exit 0\n"
  "fi\n"
  "\n"
  "# Report start of test suite to Teamcity\n"
  "  echo \"##teamcity[testSuiteStarted  name='Valgrind']\"\n"
  "\n"
  "# Read it line by line to report the failure\n"
  "while IFS= read -r line; do\n"
  "\n"
  "  id=$(cut -d':' -f1 <<< $line)\n"
  "  test=$(cut -d':' -f2 <<< $line)\n"
  "\n"
  "  # Get the file containing the failure details\n"
  "  details=$(find ./Testing -type f -name MemoryChecker.$id.log)\n"
  "\n"
  "  # Escape text for Teamcity\n"
  "  text=\$(cat $details)\n"
  "  text=\${text//|/||}\n"
  "  text=\${text//[/|[}\n"
  "  text=\${text//]/|]}\n"
  "  text=\${text//\\'/|\\'}\n"
  "  text=$(echo \"$text\" | tr '\\n' '@'| sed 's/@/|n/g')\n"
  "\n"
  "  echo \"##teamcity[testStarted name='$test']\"\n"
  "  #echo $(cat $details)\n"
  "  echo \"##teamcity[testFailed name='$test' message='Valgrind failure' details='$text']\"\n"
  "  echo \"##teamcity[testFinished name='$test']\"\n"
  "done < $FILE\n"
  "\n"
  "# Report end of test suite to Teamcity\n"
  "  echo \"##teamcity[testSuiteFinished  name='Valgrind']\"\n"
  "\n"
  "exit 0"
)

add_custom_target(report_valgrind
  COMMAND chmod +x ./report_valgrind_teamcity.sh
  COMMAND sh -c ./report_valgrind_teamcity.sh
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Reporting Memcheck result from ctest to Teamcity"
  VERBATIM
  USES_TERMINAL
)
