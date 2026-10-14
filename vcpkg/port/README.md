# Usage

## Vcpkg configuration

Add ``onbings-vista`` to

- `vcpkg.json`
- `vcpkg-configuration.json`

## CMake Integration

Add the following lines to your CMakeLists.txt

```cmake
# Find the package
find_package(onbings-vista REQUIRED)

# Link with the library
target_link_library(YOUR_TARGET PRIVATE ONBINGS::onbings-vista)
```
