# DetectToolset.cmake

This module is responsible for detecting OS and toolset and providing additional compiler flags

## Variables

This module defines the following variable

| CMake variable           | Description                                                       | Example
| -------------------------| ------------------------------------------------------------------|--------
| OS_DISTRO                | The string representing the OS distribution                       | centos
| OS_VERSION               | The string representing the version of the OS                     | 9.3
| TOOLSET_VERSION          | The string representing the version of the detected toolset       | 1
| TOOLSET_COMPILER_ARCH    | The string representing the architecture of the detected compiler | x86_64-redhat-linux
| TOOLSET_COMPILER_VERSION | The string representing the version of the detected compiler      | 13

## Compiler flags

Based on the detected OS and toolset, additional compiler are provided.

 * [GCC](../../cmake/Toolset/default/compile_flags.cmake)
 * [MSVC](../../cmake/Toolset/Windows/compile_flags.cmake)
