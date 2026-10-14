#
# Description
#   This function parses the file /etc/centos-release
#   if it exists to retrieve the full version number
#   of the centos system
#
# Parameters
#   _VERSION - The version number placeholder
#
function(read_centos_release _VERSION)

  # File /etc/centos-release contains something like this
  # "CentOS Linux release 7.5.1804 (Core)"

  if(EXISTS "/etc/centos-release")

    file(STRINGS /etc/centos-release version REGEX "[0-9]")
    string(REGEX REPLACE "[ A-Za-z()]" "" version ${version})

    set(${_VERSION} ${version} PARENT_SCOPE)

  endif()

endfunction()

#
# Description
#   This function parses the file /etc/os-release
#   if it exists to retrieve the distribution and
#   version number of the OS
#
# Parameters
#   _DISTRO  - The distribution placeholder
#   _VERSION - The version number placeholder
#
function(read_etc_osrelease _DISTRO _VERSION)

  # File /etc/os-release is something like this
  #
  # NAME="CentOS Linux"
  # VERSION="7 (Core)"
  # ID="centos"
  # ID_LIKE="rhel fedora"
  # VERSION_ID="7"
  # PRETTY_NAME="CentOS Linux 7 (Core)"
  # ...

  if(EXISTS "/etc/os-release")

    file(STRINGS /etc/os-release distro  REGEX "^ID=")
    file(STRINGS /etc/os-release version REGEX "^VERSION_ID=")

    string(REGEX REPLACE "ID=(.*)"         "\\1" distro  "${distro}")
    string(REGEX REPLACE "VERSION_ID=(.*)" "\\1" version "${version}")

    string(REPLACE "\"" "" distro  "${distro}")
    string(REPLACE "\"" "" version "${version}")

    set(${_DISTRO}  ${distro} PARENT_SCOPE)
    set(${_VERSION} ${version} PARENT_SCOPE)

  endif()

endfunction()

#
# Description
#   This function retrieves the distribution
#   and version number of the OS
#
# Parameters
#   _DISTRO  - The distribution placeholder
#   _VERSION - The version number placeholder
#
function(get_os_info _DISTRO _VERSION)

  set(distro  "")
  set(version "")

  read_etc_osrelease(distro version)

  # Centos version is more specific
  # in /etc/centos-release
  if("${distro}" STREQUAL "centos")
    read_centos_release(version)
  endif()

  # Report values
  if("${distro}" STREQUAL "")
    set(${_DISTRO} "${_DISTRO}-NOTFOUND" PARENT_SCOPE)
  else()
    set(${_DISTRO} ${distro} PARENT_SCOPE)
  endif()

  if("${version}" STREQUAL "")
    set(${_VERSION} "${_VERSION}-NOTFOUND" PARENT_SCOPE)
  else()
    set(${_VERSION} ${version} PARENT_SCOPE)
  endif()

endfunction()

# Retrieve OS information
if(WIN32)
  set(OS_DISTRO  "Windows")
  set(OS_VERSION "${CMAKE_SYSTEM_VERSION}")
else()
  get_os_info(OS_DISTRO OS_VERSION)
endif()

# Override if environment variables exist
if(DEFINED ENV{OS_NAME})
  set(OS_DISTRO "$ENV{OS_NAME}")
endif()

if(DEFINED ENV{OS_VERSION})
  set(OS_VERSION "$ENV{OS_VERSION}")
endif()

# Load the toolset
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/Toolset/${OS_DISTRO}/Toolset.cmake)
  include(${CMAKE_CURRENT_LIST_DIR}/Toolset/${OS_DISTRO}/Toolset.cmake)
else()
  include(${CMAKE_CURRENT_LIST_DIR}/Toolset/default/Toolset.cmake)
endif()

# Make sure the proper variables are defined
if(NOT DEFINED TOOLSET_VERSION)
  message(FATAL_ERROR "Your toolset version (TOOLSET_VERSION) is not defined. Please check ${CMAKE_CURRENT_LIST_DIR}/Toolset")
endif()

if(NOT DEFINED TOOLSET_COMPILER_ARCH)
  message(FATAL_ERROR "Your compiler architecture (TOOLSET_COMPILER_ARCH) is not defined. Please check ${CMAKE_CURRENT_LIST_DIR}/Toolset")
endif()

if(NOT DEFINED TOOLSET_COMPILER_VERSION)
  message(FATAL_ERROR "Your compiler version (TOOLSET_COMPILER_VERSION) is not defined. Please check ${CMAKE_CURRENT_LIST_DIR}/Toolset")
endif()
