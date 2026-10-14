# Vcpkg.cmake

This module provides facilities to related to vcpkg

## Installation

If CMake option **INSTALL_VCPKG_DEPENDENCIES** is enabled, it adds a target **${PROJECT_NAME}-vcpkg** to install vcpkg libraries

```
$>cmake --install . --prefix install_dir --target my-project-vcpkg
```

## Listing

It produces **${CMAKE_BINARY_DIR}/vcpkg-dependencies.json** that lists all dependencies that were installed using vcpkg.

This file can potentially later be used with [pin-version.py](https://bitbucket.evs.tv/projects/VCPKG/repos/evs-vcpkg-registry/browse/scripts/pin-version.py) script to pin version in "offline" mode.