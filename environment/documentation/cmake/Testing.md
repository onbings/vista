# Testing.cmake

This module provides facilities (coverage, memory checking, ...) for CTest

## Code coverage

To enable this functionality the following conditions must be met

* [CMAKE_CXX_COMPILER_ID](https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html) must be "GNU"
* The CMake option **ENABLE_COVERAGE** must be enabled

If enabled,

* A target **init_coverage** is created
* A target **collect_coverage** is created
* The compiler flag "--coverage" is globally added

Results are stored in ${CMAKE_BINARY_DIR}/Coverage/

### Usage

```bash
$>cmake --build .                           # Build the project
$>cmake --build . --target init_coverage    # Initialize coverage baseline
$>ctest .                                   # Run tests
$>cmake --build . --target collect_coverage # Collect coverage results
```

### Coverage exclusion

It might happen that some files need to be excluded of coverage computation.

The module will check for additional exclusion patterns by looking at

* The CMake variable **EXTRA_COVERAGE_EXCLUSION**
* Each file named "coverage.exclusion" located inside **CMAKE_SOURCE_DIR** tree

## Memory checking

To enable this functionality the following condition must be met

* [valgrind](https://valgrind.org/) program must be found on the system

If enabled, it sets the following standard CMake variables

* [MEMORYCHECK_COMMAND](https://cmake.org/cmake/help/latest/variable/CTEST_MEMORYCHECK_COMMAND.html)
* [MEMORYCHECK_COMMAND_OPTIONS](https://cmake.org/cmake/help/latest/variable/CTEST_MEMORYCHECK_COMMAND_OPTIONS.html)
* [MEMORYCHECK_SUPPRESSIONS_FILE](https://cmake.org/cmake/help/latest/variable/CTEST_MEMORYCHECK_SUPPRESSIONS_FILE.html)

### Usage

```bash
$> ctest -T Memcheck .
```

### Suppressions

It might happen that Valgrind reports false positives that need to be suppressed.

To do so, suppression content need to be added to [MEMORYCHECK_SUPPRESSIONS_FILE](https://cmake.org/cmake/help/latest/variable/CTEST_MEMORYCHECK_SUPPRESSIONS_FILE.html)

Example
```
add_library(my-target
  ...
)

if(MEMORYCHECK_SUPPRESSIONS_FILE)
  file(READ   valgrind_suppression.txt           CONTENT)
  file(APPEND ${MEMORYCHECK_SUPPRESSIONS_FILE} ${CONTENT})
endif()
```