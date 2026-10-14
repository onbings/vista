# Retrieve the sources from git at a specific revision
vcpkg_from_git(
        OUT_SOURCE_PATH SOURCE_PATH
        URL <<url>>
        REF <<hash>>
)
vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
        FEATURES
#        audio ONBINGS_VISTA_ADD_AUDIO
)
# CMake configure : disable optional artifacts
vcpkg_configure_cmake(
        SOURCE_PATH ${SOURCE_PATH}
        PREFER_NINJA
        OPTIONS
        -DONBINGS_VISTA_BUILD_TESTS=OFF
        -DFULL_VERSION=${VERSION}
        ${FEATURE_OPTIONS}
)

# CMake install
vcpkg_install_cmake()

# Fix installation path according to vcpkg conventions
vcpkg_fixup_cmake_targets(CONFIG_PATH share TARGET_PATH "share/${PORT}")
vcpkg_copy_pdbs()

# Remove useless directories
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

# Make sure there is a copyright file
file(WRITE ${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright "")

# Write usage file
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION ${CURRENT_PACKAGES_DIR}/share/${PORT})
