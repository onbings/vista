# ===========
# == CPACK ==
# ===========

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "ONBINGS")
set(CPACK_PACKAGE_DESCRIPTION "The installer package of ${SOLUTION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${CPACK_PACKAGE_DESCRIPTION})

# Create output file name in the given directory
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}")
set(CPACK_PACKAGE_DIRECTORY ${CMAKE_BINARY_DIR}/package)

# Only support tar.gz
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

if(UNIX)
  set(CPACK_GENERATOR "TGZ")
else()
  set(CPACK_GENERATOR "ZIP")
endif()

# Allow archive to do component based install
set(CPACK_ARCHIVE_COMPONENT_INSTALL 1)

# Install only those components in this archive
set(CPACK_COMPONENTS_ALL onbings-vista-runtime onbings-vista-vcpkg)
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

# Allow compression in //
set(CPACK_THREADS 0)

# Try to build relocatable packages
set(CPACK_PACKAGE_RELOCATABLE TRUE)

# Always the last directive
include(CPack)
