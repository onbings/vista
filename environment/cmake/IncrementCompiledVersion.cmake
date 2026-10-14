if(NOT DEFINED HEADER_PATH)
  message(FATAL_ERROR "HEADER_PATH is required")
endif()

if(NOT DEFINED VERSION_PREFIX)
  set(VERSION_PREFIX "0.0.0")
endif()

if(NOT DEFINED REPO_ROOT)
  set(REPO_ROOT "")
endif()

if(NOT EXISTS "${HEADER_PATH}")
  message(FATAL_ERROR "Header file not found: ${HEADER_PATH}")
endif()

file(READ "${HEADER_PATH}" HEADER_CONTENT)

set(BUILD_COUNTER 0)
string(REGEX MATCH "get_version_build_str[ \t]*\\(\\)[ \t]*\\{[ \t]*return[ \t]*\"([0-9]+)\";" BUILD_MATCH "${HEADER_CONTENT}")
if(CMAKE_MATCH_1)
  set(BUILD_COUNTER "${CMAKE_MATCH_1}")
endif()

math(EXPR BUILD_COUNTER "${BUILD_COUNTER} + 1")
math(EXPR BUILD_COUNTER "${BUILD_COUNTER} % 100000")

string(LENGTH "${BUILD_COUNTER}" BUILD_LEN)
if(BUILD_LEN LESS 5)
  math(EXPR PADDING_LEN "5 - ${BUILD_LEN}")
  string(REPEAT "0" ${PADDING_LEN} PADDING)
  set(BUILD_PADDED "${PADDING}${BUILD_COUNTER}")
else()
  set(BUILD_PADDED "${BUILD_COUNTER}")
endif()

set(VERSION_BASE "${VERSION_PREFIX}")
string(REGEX MATCH "get_version[ \t]*\\(\\)[ \t]*\\{[ \t]*return[ \t]*\"([0-9]+\\.[0-9]+\\.[0-9]+)\\.[0-9]+(\\+[^\"]*)?\";" VERSION_MATCH "${HEADER_CONTENT}")
if(CMAKE_MATCH_1)
  set(VERSION_BASE "${CMAKE_MATCH_1}")
endif()

set(GIT_SHORT_SHA "")
if(NOT REPO_ROOT STREQUAL "")
  execute_process(
    COMMAND git -C "${REPO_ROOT}" rev-parse --short=7 HEAD
    OUTPUT_VARIABLE GIT_SHORT_SHA
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

set(BUILD_USER "$ENV{USER}")
if(BUILD_USER STREQUAL "")
  set(BUILD_USER "$ENV{USERNAME}")
endif()

if(NOT GIT_SHORT_SHA STREQUAL "")
  string(REGEX REPLACE "[^0-9A-Za-z-]" "-" GIT_SHORT_SHA "${GIT_SHORT_SHA}")
endif()
if(NOT BUILD_USER STREQUAL "")
  string(REGEX REPLACE "[^0-9A-Za-z-]" "-" BUILD_USER "${BUILD_USER}")
endif()

set(BUILD_METADATA_PARTS "")
if(NOT GIT_SHORT_SHA STREQUAL "")
  list(APPEND BUILD_METADATA_PARTS "${GIT_SHORT_SHA}")
endif()
if(NOT BUILD_USER STREQUAL "")
  list(APPEND BUILD_METADATA_PARTS "${BUILD_USER}")
endif()

set(BUILD_METADATA_SUFFIX "")
if(BUILD_METADATA_PARTS)
  string(REPLACE ";" "." BUILD_METADATA "${BUILD_METADATA_PARTS}")
  set(BUILD_METADATA_SUFFIX "+${BUILD_METADATA}")
endif()

set(FULL_VERSION "${VERSION_BASE}.${BUILD_PADDED}${BUILD_METADATA_SUFFIX}")

string(REGEX REPLACE
  "get_version_build_str[ \t]*\\(\\)[ \t]*\\{[ \t]*return[ \t]*\"[0-9]+\";"
  "get_version_build_str () { return \"${BUILD_PADDED}\";"
  HEADER_CONTENT
  "${HEADER_CONTENT}"
)

string(REGEX REPLACE
  "get_version[ \t]*\\(\\)[ \t]*\\{[ \t]*return[ \t]*\"[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+(\\+[^\"]*)?\";"
  "get_version           () { return \"${FULL_VERSION}\";"
  HEADER_CONTENT
  "${HEADER_CONTENT}"
)

file(WRITE "${HEADER_PATH}" "${HEADER_CONTENT}")
message(STATUS "Incremented compiled version to ${FULL_VERSION}")
