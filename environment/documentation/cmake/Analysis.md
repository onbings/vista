# Analysis.cmake

This module is responsible for configuring code analyzer

## clang-tidy

To enable this functionality, the following conditions must be met

* [clang-tidy](https://clang.llvm.org/extra/clang-tidy/) program must be found on the system
* CMake option **ENABLE_CLANGTIDY** should be enabled

When enabled, it sets the CMake standard variable [CMAKE_CXX_CLANG_TIDY](https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_CLANG_TIDY.html)

## include-what-you-use

To enable this functionality, the following conditions must be met

* [include-what-you-use](https://github.com/include-what-you-use/include-what-you-use) program must be found on the system
* CMake option **ENABLE_IWYU** should be enabled

When enabled, it sets CMake standard variable [CMAKE_CXX_INCLUDE_WHAT_YOU_USE](https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_INCLUDE_WHAT_YOU_USE.html)