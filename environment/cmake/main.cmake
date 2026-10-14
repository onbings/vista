
# ===========================
# == DEFAULT CONFIGURATION ==
# ===========================

include(${CMAKE_CURRENT_LIST_DIR}/DefaultConfigurations.cmake)

# =============
# == OPTIONS ==
# =============

include(${CMAKE_CURRENT_LIST_DIR}/Options.cmake)

# =========================
# == TOOLSET INFORMATION ==
# =========================

include(${CMAKE_CURRENT_LIST_DIR}/DetectToolset.cmake)

# ================================
# == SOURCE CONTROL INFORMATION ==
# ================================

include(${CMAKE_CURRENT_LIST_DIR}/SourceControl.cmake)

# ================
# == VERSIONING ==
# ================

include(${CMAKE_CURRENT_LIST_DIR}/Versioning.cmake)

# =============
# == TESTING ==
# =============

include(${CMAKE_CURRENT_LIST_DIR}/Testing.cmake)

# ==============
# == TEAMCITY ==
# ==============

# This needs to be after "Versioning"
include(${CMAKE_CURRENT_LIST_DIR}/Teamcity.cmake)

# ==============
# == ANALYSIS ==
# ==============

include(${CMAKE_CURRENT_LIST_DIR}/Analysis.cmake)

# ===========
# == VCPKG ==
# ===========

include(${CMAKE_CURRENT_LIST_DIR}/Vcpkg.cmake)

# ==============
# == APPIMAGE ==
# ==============

include(${CMAKE_CURRENT_LIST_DIR}/AppImage.cmake)

# ============  
# == PYTHON ==
# ============

include(${CMAKE_CURRENT_LIST_DIR}/Python.cmake)

# ===========
# == RPATH ==
# ===========

if(DEFINED CMAKE_SYSROOT)
  message(STATUS "Adding sysroot (${CMAKE_SYSROOT}/lib) to RPATH")
  list(APPEND CMAKE_BUILD_RPATH "${CMAKE_SYSROOT}/lib")
endif()

# ===================
# == PRINT SUMMARY ==
# ===================

message(STATUS "Build information")
message(STATUS "-----------------")
message(STATUS "Build type         : ${CMAKE_BUILD_TYPE}")
message(STATUS "Distribution       : ${OS_DISTRO}")
message(STATUS "Compiler arch      : ${TOOLSET_COMPILER_ARCH}")
message(STATUS "Compiler version   : ${TOOLSET_COMPILER_VERSION}")
message(STATUS "Toolset version    : ${TOOLSET_VERSION}")
message(STATUS "OS Version         : ${OS_VERSION}")
message(STATUS "Source control     : ${SOURCE_CONTROL_TYPE}")
message(STATUS "Branch             : ${SOURCE_CONTROL_BRANCH}")
message(STATUS "Commit             : ${SOURCE_CONTROL_COMMIT_SHORT} (${SOURCE_CONTROL_COMMIT_FULL})")
message(STATUS "URL                : ${SOURCE_CONTROL_URL}")
message(STATUS "Package version    : ${PROJECT_VERSION}")
message(STATUS "Valgrind           : ${MEMORYCHECK_COMMAND} (${MEMORYCHECK_VERSION})")

if(LINUXDEPLOY_COMMAND)
  message(STATUS "Linuxdeploy        : ${LINUXDEPLOY_COMMAND} (${LINUXDEPLOY_VERSION})")
else()
  message(STATUS "Linuxdeploy        : Not available")
endif()

if(ENABLE_CLANGTIDY)
  message(STATUS "clang-tidy         : ${CMAKE_CXX_CLANG_TIDY} (${CLANGTIDY_VERSION})")
else()
  message(STATUS "clang-tidy         : Disabled")
endif()

if(ENABLE_IWYU)
  message(STATUS "iwyu               : ${CMAKE_CXX_INCLUDE_WHAT_YOU_USE} (${IWYU_VERSION})")
else()
  message(STATUS "iwyu               : Disabled")
endif()

if(TARGET init_coverage)
  message(STATUS "Code coverage      : Enabled")
else()
  message(STATUS "Code coverage      : Not available")
endif()

if(BUILD_SHARED_LIBS)
  message(STATUS "Libraries          : shared")
else()
  message(STATUS "Libraries          : static")
endif()

if(ENABLE_VERSIONING)
  message(STATUS "Versioning         : Enabled")
else()
  message(STATUS "Versioning         : Disabled")
endif()

if(ENABLE_BUILD_PYTHON_WHEEL)
  message(STATUS "Python build wheel : Enabled")
  message(STATUS "Python version     : ${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}")
  message(STATUS "Python executable  : ${Python_EXECUTABLE}")
else()
  message(STATUS "Python build wheel : Disabled")
endif()

message("")
