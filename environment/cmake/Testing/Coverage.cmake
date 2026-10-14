# By default enable coverage on Debug build
string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)

if("${BUILD_TYPE_LOWER}" STREQUAL "debug")
  set(DEFAULT_ENABLE_COVERAGE ON)
else()
  set(DEFAULT_ENABLE_COVERAGE OFF)
endif()

# Options
option(ENABLE_COVERAGE          "Enable code coverage" ${DEFAULT_ENABLE_COVERAGE})
set   (EXTRA_COVERAGE_EXCLUSION "")

# Catch all exclusions files
file(GLOB_RECURSE EXCLUSION_FILES ${CMAKE_SOURCE_DIR}/coverage.exclusion)

foreach(file ${EXCLUSION_FILES})

  # Get directory
  cmake_path(GET file PARENT_PATH directory)

  # Read file line by line
  file(STRINGS ${file} items)
  
  foreach(item ${items})
    # Append relative path to coverage.exclusion directory
    string    (REPLACE "\\" "/" item "${item}")
    cmake_path(APPEND directory ${item} OUTPUT_VARIABLE exclusion)
    cmake_path(NORMAL_PATH exclusion)
    
    # Add it to the exclusion list
    set(EXTRA_COVERAGE_EXCLUSION "${EXTRA_COVERAGE_EXCLUSION} \'${exclusion}/*\'")
  endforeach()
  
endforeach()

#message(STATUS "EXTRA_COVERAGE_EXCLUSION : ${EXTRA_COVERAGE_EXCLUSION}")

if(ENABLE_COVERAGE)

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    include(${CMAKE_CURRENT_LIST_DIR}/GnuCoverage.cmake)
  else()
    message(WARNING "No supported coverage strategy for this compiler : ${CMAKE_CXX_COMPILER_ID}")
  endif()

endif()
