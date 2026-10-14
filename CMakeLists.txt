cmake_minimum_required(VERSION 3.23)

# Paranoid check : in-source generation is forbidden
if("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "You cannot generate your binary solution into your source directory !")
endif()

include(CMakeDependentOption)

# Define options to enable/disable non-necessary artifacts
option(ONBINGS_VISTA_BUILD_TESTS "Build units tests of onbings-vista" ON)
option(ONBINGS_VISTA_FORCE_EXPORT_ALL "Force exporting all symbols of onbings-vista (for shared library)" OFF)
option(ONBINGS_VISTA_BUILD_DOC_HTML "Build HTML documentation using Doxygen" OFF)
option(ONBINGS_VISTA_BUILD_DOC_PDF "Build PDF documentation using Doxygen" OFF)


if(ONBINGS_VISTA_BUILD_TESTS)
  list(APPEND VCPKG_MANIFEST_FEATURES "gtest")
endif()
project(onbings-vista VERSION 1.2.3)

# Adjust version if needed
if(PROJECT_IS_TOP_LEVEL)
  if(DEFINED FULL_VERSION)
    set(PROJECT_VERSION ${FULL_VERSION})
    set(CMAKE_PROJECT_VERSION ${FULL_VERSION})
  elseif(DEFINED BUILD_NUMBER)
    set(PROJECT_VERSION_TWEAK ${BUILD_NUMBER})
    set(PROJECT_VERSION ${PROJECT_VERSION}.${BUILD_NUMBER})
    set(CMAKE_PROJECT_VERSION_TWEAK ${PROJECT_VERSION_TWEAK})
    set(CMAKE_PROJECT_VERSION ${PROJECT_VERSION})
  endif()
endif()

find_package(evs-cmake REQUIRED)
message(STATUS "TeamCity Build Config: '$ENV{TEAMCITY_BUILDCONF_NAME}'")
message(STATUS "TeamCity Project Name: '$ENV{TEAMCITY_PROJECT_NAME}'")

if(DEFINED ENV{TEAMCITY_VERSION})
  add_definitions(-DTEAMCITY_BUILD)
  # Detect platform from build config name
  if(DEFINED ENV{TEAMCITY_BUILDCONF_NAME})
    message(STATUS "TeamCity Build Config: $ENV{TEAMCITY_BUILDCONF_NAME}")
    if("$ENV{TEAMCITY_BUILDCONF_NAME}" MATCHES "p6x")
      message(STATUS "TeamCity Build Config under P6X")
      add_definitions(-DTEAMCITY_P6X_BUILD)
    elseif("$ENV{TEAMCITY_BUILDCONF_NAME}" MATCHES "rocky_9")
      message(STATUS "TeamCity Build Config under ROCKY9")
      add_definitions(-DTEAMCITY_ROCKY9_BUILD)
      # Check if this is a night build
      if("$ENV{TEAMCITY_BUILDCONF_NAME}" MATCHES "Nightly")
        message(STATUS "TeamCity Build Config under NIGHTBUILD") #Both flag defined in this case
        add_definitions(-DTEAMCITY_NIGHT_BUILD)
      endif()
    endif()
  endif()
endif()

# We are the root directory
if("x${CMAKE_SOURCE_DIR}x" STREQUAL "x${CMAKE_CURRENT_SOURCE_DIR}x")
  if(EXISTS ${CMAKE_SOURCE_DIR}/environment/cmake/main.cmake)
    include(${CMAKE_SOURCE_DIR}/environment/cmake/main.cmake)
  else()
    message(STATUS "Could not find environment directory. Did you forgot to run \"git submodule update --init\" ?")
  endif()
endif()

# Check options that needs to come 'after' include(environment/cmake/main.cmake)
cmake_dependent_option(ONBINGS_VISTA_BUILD_APPIMAGES "Build appimages for applications of onbings-vista" ON "LINUXDEPLOY_COMMAND" OFF)
#cmake_dependent_option(ONBINGS_VISTA_BUILD_APPIMAGES "Build appimages for applications of onbings-vista" OFF "LINUXDEPLOY_COMMAND" OFF)
#set(ONBINGS_VISTA_BUILD_APPIMAGES OFF)
message(STATUS "Options")
message(STATUS "  ONBINGS_VISTA_BUILD_TESTS      = ${ONBINGS_VISTA_BUILD_TESTS}")
message(STATUS "  ONBINGS_VISTA_FORCE_EXPORT_ALL = ${ONBINGS_VISTA_FORCE_EXPORT_ALL}")
message(STATUS "  ONBINGS_VISTA_BUILD_APPIMAGES  = ${ONBINGS_VISTA_BUILD_APPIMAGES}")
message(STATUS "  ONBINGS_VISTA_BUILD_DOC_HTML   = ${ONBINGS_VISTA_BUILD_DOC_HTML}")
message(STATUS "  ONBINGS_VISTA_BUILD_DOC_PDF    = ${ONBINGS_VISTA_BUILD_DOC_PDF}")

# ===============================
# == NON TRANSITIVE PROPERTIES ==
# ===============================

# Non transitive properties are properties that are not
# usage requirements. It means properties that only need
# to be set for building this target only. As such they
# don't need to propagated outside.
# For convenience, use global initializer for those

# Disable compiler extension to maximize portability
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# For shared libraries we need to compile with "-fPIC"
# But for static libraries user can choose
if(NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
  set(CMAKE_POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})
endif()

# API visibility control
if(ONBINGS_VISTA_FORCE_EXPORT_ALL)

  if(MSVC AND BUILD_SHARED_LIBS)

    message(WARNING "\nYou requested to force exporting ALL symbols "
      "in a Windows DLL through CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS\n"
      "This is a hack that has limitations. Use it as a temporary workaround")
  endif()

  # By "default",
  #  - GCC and Clang export everything (i.e. nothing to do)
  #  - MSVC hides everything and requires explicit listing
  #
  # This command tells CMake to generate such listing
  # for all the symbols found in the binary
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

else()

  # We only want the API symbols to be visible
  # Tell CMake to pass the proper flags to hide
  # everything that is not explicitly marked
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)
  set(CMAKE_CXX_VISIBILITY_INLINES_HIDDEN ON)

endif()

# This creates the 'test' target
# even if you don't have tests
enable_testing()

# ==============================
# == PRESET INSTALL VARIABLES ==
# ==============================

# This include default installation directories
# based on detection performed by CMake. They
# are just default that can be overriden by the user.
include(GNUInstallDirs)

# It creates for example the following variables
#  ${CMAKE_INSTALL_BINDIR}      --> bin
#  ${CMAKE_INSTALL_LIBDIR}      --> lib, lib64, etc, ...
#  ${CMAKE_INSTALL_INCLUDEDIR}  --> include
#
# All those paths are relative to ${CMAKE_INSTALL_PREFIX} that is not
# known at configuration time and can be overriden by the user when installing.
#
# For platforms that support RPATH and ORIGIN, we can then compute the
# relative path from ${CMAKE_INSTALL_BINDIR} to ${CMAKE_INSTALL_PREFIX}
# example :
#   bin           --> some/weird/path
#   relative path --> ../../../
#
# and override RPATH with $ORIGIN/RELATIVE_PATH/${CMAKE_INSTALL_LIBDIR} at installation
#
file(RELATIVE_PATH BIN_TO_BASE_INSTALL_PATH ${CMAKE_CURRENT_LIST_DIR}/${CMAKE_INSTALL_BINDIR} ${CMAKE_CURRENT_LIST_DIR})
set(RELATIVE_PATH_BIN_TO_LIB ${BIN_TO_BASE_INSTALL_PATH}${CMAKE_INSTALL_LIBDIR})

# =============================
# == SETUP BUILD DIRECTORIES ==
# =============================

# Pre-stage binaries like if they were already installed
#  - CMAKE_LIBRARY_OUTPUT_DIRECTORY is for shared libraries
#  - CMAKE_ARCHIVE_OUTPUT_DIRECTORY is for static libraries
#  - CMAKE_RUNTIME_OUTPUT_DIRECTORY is for applications
set(COMPILED_BINARIES_DIR ${CMAKE_BINARY_DIR}/binaries)
set(BASE_RESOURCES_DIR ${CMAKE_BINARY_DIR}/resources)
set(APPIMAGES_DIR ${CMAKE_BINARY_DIR}/appimages)

if(WIN32)
  set(LIBRARY_OUTPUT_DIRECTORY ${COMPILED_BINARIES_DIR}/bin)
  set(RUNTIME_OUTPUT_DIRECTORY ${COMPILED_BINARIES_DIR}/bin)
else()
  set(LIBRARY_OUTPUT_DIRECTORY ${COMPILED_BINARIES_DIR}/lib)
  set(RUNTIME_OUTPUT_DIRECTORY ${COMPILED_BINARIES_DIR}/bin)
endif()

set(ARCHIVE_OUTPUT_DIRECTORY ${COMPILED_BINARIES_DIR}/lib)

# We might not be the root project, so if root project
# already defined them, do not override
if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_DIRECTORY})
endif()

if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${RUNTIME_OUTPUT_DIRECTORY})
endif()

if(NOT DEFINED CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${ARCHIVE_OUTPUT_DIRECTORY})
endif()

# ==================================
# == POPULATE CONFIGURATION FILES ==
# ==================================

# Add configuration files to a dummy library to
# make sure files are properly found
add_library(onbings-vista-config-files INTERFACE
  .gitignore
  CMakePresets.json
  Config.cmake.in
  Dependencies.cmake.in
  README.md
  vcpkg-configuration.json
  vcpkg.json
)

# ===========================
# == EXTERNAL DEPENDENCIES ==
# ===========================

# We need to perform some "find_package" to find out our direct dependencies.
#
# Moreover, other projects that will depend on us will also have to perform
# those find_package in order to transitively link with our own dependencies.
#
# By isolating them in a file, we fulfill a double objective
#  1°) We can simply include it to find the dependencies we need
#  2°) We can later install that file and include it from our configuration
#      file. As such, our dependees will automatically transitively find
#      our dependencies
#
# Remark 1
# --------
#  As always, we might have 2 types of dependencies.
#    1°) The ones needed to link with our libraries (i.e. usage requirements)
#    2°) The ones only needed to build              (i.e. build requirements)
#
# Remark 2
# --------
#  ALL dependencies used by the library (public AND private) should be referenced in the public
#  dependency file (Dependencies.cmake). Because,
#  1°) When compiled as a static library, even private libraries needs to be supplied to the linker.
#  2°) To run unit tests, we might still need to have found private libraries

# Configure the dependency file according to options
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Dependencies.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/onbings-vista-dependencies.cmake
  @ONLY
)

include(${CMAKE_CURRENT_BINARY_DIR}/onbings-vista-dependencies.cmake)

# =======================
# == POPULATE PROJECTS ==
# =======================

# Mandatory
add_subdirectory(code)

# ==========================
# == PACKAGE INSTALLATION ==
# ==========================

# Export our targets
install(
  EXPORT ONBINGS_VISTA
  FILE onbings-vista-targets.cmake
  NAMESPACE ONBINGS::
  DESTINATION ${CMAKE_INSTALL_DATADIR}
  COMPONENT onbings-vista-devel
)

# Create configuration file so that find_package() from
# our dependees can find us
include(CMakePackageConfigHelpers)

set(INTERNAL_NAMESPACE EVS)

# Create and install the cmake configuration file based on our template
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/onbings-vista-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_DATADIR}
)

unset(INTERNAL_NAMESPACE)

# Generate the version file and indicate compatibility strategy
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/onbings-vista-config-version.cmake
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)

# Install them
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/onbings-vista-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/onbings-vista-config-version.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/onbings-vista-dependencies.cmake
  DESTINATION ${CMAKE_INSTALL_DATADIR}
  COMPONENT onbings-vista-devel
)

# ===============================
# == DOCUMENTATION GENERATION ==
# ===============================

# Check if documentation generation is requested
if(ONBINGS_VISTA_BUILD_DOC_HTML OR ONBINGS_VISTA_BUILD_DOC_PDF)
  # Build argument list for the function call
  set(DOC_ARGS "")
  if(ONBINGS_VISTA_BUILD_DOC_HTML)
    list(APPEND DOC_ARGS "BUILD_HTML")
  endif()
  if(ONBINGS_VISTA_BUILD_DOC_PDF)
    list(APPEND DOC_ARGS "BUILD_PDF")
  endif()

  evs_generate_doxygen_documentation(
    ${DOC_ARGS}
    PROJECT_NAME "ONBINGS VISTA SERVICE"
    PROJECT_BRIEF "P6x Realtime Interface (for) Stream Monitoring"
    PROJECT_VERSION "${PROJECT_VERSION}"
    INPUT_FILES "${CMAKE_CURRENT_SOURCE_DIR}/code/sources/onbings-vista-lib/include/onbings-vista/vista_engine.h"
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/doc"
    TARGET_NAME "onbings-vista-doc"
    INSTALL_COMPONENT "onbings-vista-doc"
  )
endif()

include(packaging.cmake)
