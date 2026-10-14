# AppImage.cmake

This module offers facilities to create AppImage

## Enabling

To enable this feature, the following condition(s) must be met

* [linuxdeploy](https://github.com/linuxdeploy/linuxdeploy) program must be found on the system

## CMake variables

| CMake variable      | Description
| ------------------- | -----------
| LINUXDEPLOY_COMMAND | The path to the [linuxdeploy](https://github.com/linuxdeploy/linuxdeploy) executable

This variable can be used to initialize dependent option values

```
cmake_dependent_option(MY_PROJECT_BUILD_APPIMAGES "Build appimages for application" ON "LINUXDEPLOY_COMMAND" OFF)
```

## CMake function

When enabled, the following function is available

```
generate_appimage(TARGET         <target>
                  [ICON          <file>]
                  [DESKTOP_FILE  <file>]
                  [CUSTOM_APPRUN <file>]
                  [COPY_FILE     <file_1> <rel_path_1>]
                  [COPY_FILE     <file_2> <rel_path_2>]
                  ...
                  [COPY_DIR      <dir_1>  <rel_path_1>]
                  [COPY_DIR      <dir_2>  <rel_path_2>]
                  ...
                  [OUTPUT_DIR    <dir>]
                  [EXCLUDE_SYSROOT]
)
```

| Parameter       | Description
| -------------   | -----------
| TARGET          | The CMake target of the application for which to create the AppImage
| ICON            | The path to an icon file 
| DESKTOP_FILE    | The path to a desktop file according desktop entry specification : https://specifications.freedesktop.org/desktop-entry-spec/latest/
| COPY_FILE       | A directive to copy a file inside the appimage. The destination is relative to {AppDir}/usr/
| COPY_DIR        | A directive to copy a directory inside the appimage. The destination is relative to {AppDir}/usr/
| CUSTOM_APPRUN   | A custom file to execute upon launching the appimage (e.g. startup script)
| OUTPUT_DIR      | The directory path where to output the resulting appimage
| EXCLUDE_SYSROOT | A flag to tell linuxdeploy to exclude any library originating from the sysroot directory (only effective if CMAKE_SYSROOT is set)

**Remarks**

* If no ICON is provided, an empty svg is given to linuxdeploy tool
* If no DESKTOP_FILE is provided, option "--create-desktop-file" is passed to linuxdeploy tool to create a "dummy" desktop file
* If no OUTPUT_DIR is provided, the AppImage will be created at "${CMAKE_BINARY_DIR}/appimages"

### Examples

#### Simple binary

```
add_executable(my-target
  src/main.cpp
)

if(MY_PROJECT_BUILD_APPIMAGES)
  generate_appimage(TARGET my-target)
endif()
```

### Binary with custom parameters

Let's suppose we need to pass additional parameters to our executable relative to a file enclosed in the image itself.

startup-script.sh
```bash
#!/bin/bash

# Get current directory
HERE="$(dirname "$(readlink -f "${0}")")"

# Set APPDIR when running directly from the AppDir:
if [ -z $APPDIR ]; then
    APPDIR=$(readlink -f $(dirname "$0"))
fi

exec "$HERE/usr/bin/my-target" --config "$HERE/usr/share/config.json" "$@"
```
CMakeLists.txt
```
add_executable(my-target
  src/main.cpp
)

set(RESOURCE_FILES
  resources/evs.svg
  resources/my-target.desktop
  resources/config.json
  resources/startup-script.sh
)

if(MY_PROJECT_BUILD_APPIMAGES)
  generate_appimage(
    TARGET        my-target
    ICON          ${CMAKE_CURRENT_SOURCE_DIR}/resources/evs.svg
    DESKTOP_FILE  ${CMAKE_CURRENT_SOURCE_DIR}/resources/my-target.desktop
    COPY_FILE     ${CMAKE_CURRENT_SOURCE_DIR}/resources/config.json share
    CUSTOM_APPRUN ${CMAKE_CURRENT_SOURCE_DIR}/resources/startup-script.sh
    OUTPUT_DIR    ${CMAKE_CURRENT_BINARY_DIR}/custom
    EXCLUDE_SYSROOT
  )
endif()
```