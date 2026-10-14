#include(${CMAKE_CURRENT_LIST_DIR}/vcpkg/fixup_vs_crt.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/vcpkg/vcpkg_install.cmake)

# Align msvc runtime with the one from the selected triplet
#set_msvc_runtime_library()

# Fix rpath from installed vpckg dependencies
patch_vcpkg_installed_rpath()

# Write vcpkg install dependencies manifest
list_installed_vcpkg_dependencies(${CMAKE_BINARY_DIR}/vcpkg-dependencies.json)

# Allow installing vcpkg dependencies
option(INSTALL_VCPKG_DEPENDENCIES "Add component ${PROJECT_NAME}-vcpkg to install vcpkg dependencies when calling cmake install" ON)

if(INSTALL_VCPKG_DEPENDENCIES)
  install_vcpkg_dependencies()
endif()