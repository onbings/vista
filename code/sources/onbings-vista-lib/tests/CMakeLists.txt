
include(GoogleTest)

# =============
# == SOURCES ==
# =============

set(SOURCES_FILES
  src/gtestrunner.cpp
  src/gtestrunner.h
  src/ut_vista_engine.cpp
)
# For Visual Studio
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "src" FILES ${SOURCES_FILES})

# ===========
# == BUILD ==
# ===========

# Create target
add_executable(onbings-vista-tests
  ${SOURCES_FILES}
)

# Includes
target_include_directories(onbings-vista-tests
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src>
)

# Link
target_link_libraries(onbings-vista-tests
  PRIVATE
  ONBINGS::onbings-vista-objects
  GTest::GTest
  EVS::evs-hwfw-chip
)

# Defines
target_compile_definitions(onbings-vista-tests
  PRIVATE
  # void import directive if library was shared as
  # we are actually linking with the object library
  -DONBINGS_VISTA_STATIC_DEFINE=1
)

# Add test and allow them to run in //
gtest_discover_tests(onbings-vista-tests
  XML_OUTPUT_DIR ${CMAKE_BINARY_DIR}/GTestReports/
  TEST_PREFIX onbings-vista_
  DISCOVERY_TIMEOUT 120
  DISCOVERY_MODE PRE_TEST
  WORKING_DIRECTORY $<TARGET_FILE_DIR:onbings-vista>
  PROPERTIES
  RUN_SERIAL TRUE
)

# If you plan on running your unit tests with Valgrind
# it might happen that you need to add 'suppressions'
# to silence known issues. valgrind accepts suppressions
# by taking a file (--log-file=...). The variable
# MEMORYCHECK_SUPPRESSIONS_FILE is not a CMake variable
# but can be used as a convenience to accumulate suppressions
# from all the unit tests. Consequently, if you configure
# valgrind to use that file and define this variable, you will
# have the expected result.
if(MEMORYCHECK_SUPPRESSIONS_FILE)
  file(READ valgrind_suppression.txt CONTENT)
  file(APPEND ${MEMORYCHECK_SUPPRESSIONS_FILE} ${CONTENT})
endif()
