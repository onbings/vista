#=======================================================================
#
# This file is intended to add everything related to GNU code coverage
#
#=======================================================================

if(TARGET init_coverage)
  message(STATUS "Target init_coverage already exists. Skipping")
  return()
endif()

if(TARGET collect_coverage)
  message(STATUS "Target collect_coverage already exists. Skipping")
  return()
endif()

find_program(LCOV_PATH    lcov)
find_program(GENHTML_PATH genhtml)

if(NOT LCOV_PATH)
  message(WARNING "lcov is not found though code coverage statistics is enabled ! Try Installing lcov (sudo apt-get install lcov) ...")
  return()
endif()

if(NOT GENHTML_PATH)
  message(WARNING "genhtml not found! though code coverage statistics is enabled ! Try Installing lcov (sudo apt-get install lcov) ...")
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_LOWER)

if(NOT "${BUILD_TYPE_LOWER}" STREQUAL "debug")
  message(WARNING "Requesting coverage on non Debug build (Configuration: ${CMAKE_BUILD_TYPE}) may lead to misleading results")
endif()

# Append required flags
add_compile_options(
  $<$<COMPILE_LANGUAGE:C,CXX>:--coverage>
)
add_link_options(
  $<$<COMPILE_LANGUAGE:C,CXX>:--coverage>
)

# Create the coverage exclusion list. By default
# exclude system library (in /usr) and unit tests (Unittests)

set(BIN_DIR  ${CMAKE_BINARY_DIR})

if(DOWNLOAD_DIR)
  set(REPO_DIR ${DOWNLOAD_DIR})
else()
  set(REPO_DIR ${BIN_DIR})
endif()

# Resolve potential symlinks
get_filename_component(REAL_BIN_DIR  ${BIN_DIR}  REALPATH)
get_filename_component(REAL_REPO_DIR ${REPO_DIR} REALPATH)

# General patten for exclusion
set(COVERAGE_EXCLUSION "${COVERAGE_EXCLUSION} \'*tests/*\' \'*Tests/*\' \'*example/*\' \'*tools/*\' \'*documentation/*\' \'/usr/*\' \'/opt/evs/*\' \'/opt/rh/*\' \'${BIN_DIR}/*\' \'${REPO_DIR}/*\' \'${REAL_BIN_DIR}/*\' \'${REAL_REPO_DIR}/*\'")

# Add target to collect code coverage. The principle is the following :
# 1°) At compilation time, each source code will generate coverage information
#     into a .gcno file. The init_coverage phase consist into collecting information
#     for all those file in ordre to create the "baseline"
# 2°) The unit tests are executed. During execution, they will produce .gdca files
#     that will contain information in regard to lines hit during execution
# 3°) We collect all information from those .gdca files
# 4°) We merge them with baseline collected above
# 5°) We exclude from the results all the directories we don't wish to see
#     like the code from the unit tests themselves, the third parties, etc, ...
# 6°) We generate HTML pages for a better display of the information
# 7°) We collect the overall percentage to display it in the Teamcity interface

# Create shell script to initialize coverage
file(WRITE ${CMAKE_BINARY_DIR}/init_coverage.sh
  "lcov --capture --initial --directory ./ -o ./coverage.base.info"
)

# Create shell script to collect coverage
file(WRITE ${CMAKE_BINARY_DIR}/collect_coverage.sh
  "lcov --directory ./ --capture --output-file ./coverage.test.info\n"
  "lcov --add-tracefile ./coverage.base.info --add-tracefile ./coverage.test.info -o ./coverage.info\n"
  "lcov --remove ./coverage.info ${COVERAGE_EXCLUSION} ${EXTRA_COVERAGE_EXCLUSION} --output-file ./coverage.info.cleaned\n"
  "genhtml -o ./Coverage/ ./coverage.info.cleaned\n"
  "cmake -E remove ./coverage.test.info ./coverage.info ./coverage.info.cleaned\n"
  "chmod +x ./report_coverage_teamcity.sh\n"
  "./report_coverage_teamcity.sh\n"
)

# Create file to report coverage information to Teamcity
file(WRITE ${CMAKE_BINARY_DIR}/report_coverage_teamcity.sh
  "# Extract overall coverage\n"
  "coverage=$(cat ./Coverage/index.html | grep  \"headerCovTableEntry\" | grep -m1 \"%\"  | sed -e 's/<[^>]*>//g' | tr -d \" \t\n\r\")\n"
  "# Extract summary information\n"
  "TABLE=$(cat ./Coverage/index.html | grep headerCovTableEntry\\\" | sed -e 's/[^0-9]/ /g' -e 's/^ *//g' -e 's/ *$//g')\n"
  "CoveredLine=$(echo $TABLE | awk '{print $1}')\n"
  "TotalLine=$(echo $TABLE | awk '{print $2}')\n"
  "CoveredFunction=$(echo $TABLE | awk '{print $3}')\n"
  "TotalFunction=$(echo $TABLE | awk '{print $4}')\n"
  "# Report information to Teamcity\n"
  "echo \"##teamcity[buildStatus text='{build.status.text} Coverage: $coverage']\"\n"
  "echo \"##teamcity[buildStatisticValue key='CodeCoverageAbsLCovered' value='$CoveredLine']\"\n"
  "echo \"##teamcity[buildStatisticValue key='CodeCoverageAbsLTotal' value='$TotalLine']\"\n"
  "echo \"##teamcity[buildStatisticValue key='CodeCoverageAbsMCovered' value='$CoveredFunction']\"\n"
  "echo \"##teamcity[buildStatisticValue key='CodeCoverageAbsMTotal' value='$TotalFunction']\"\n"
)

add_custom_target(init_coverage
  COMMAND chmod +x ./init_coverage.sh
  COMMAND sh -c ./init_coverage.sh
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Get baseline for code coverage"
  VERBATIM
  USES_TERMINAL
)

add_custom_target(collect_coverage
  COMMAND chmod +x ./collect_coverage.sh
  COMMAND sh -c ./collect_coverage.sh
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMENT "Processing code coverage"
)

