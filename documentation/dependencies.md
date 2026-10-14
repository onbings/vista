# Word of caution

At some point any project will need to deal with dependencies.

Though they help reducing code duplication, they are hard to manage at scale.

**Always think twice before adding a dependency !!!**

# VCPKG

When possible, favor dependencies available from vcpkg as they offer a "standard" interface to manage dependencies.

## Adding

Let's suppose we need to add [jsoncpp](https://github.com/open-source-parsers/jsoncpp)

1. Add your dependency to [vcpkg.json](../vcpkg.json)

```json
"dependencies": [
    { "name": "jsoncpp" }
  ],
```

2. Add the required [find_package](https://cmake.org/cmake/help/latest/command/find_package.html) to [Dependencies.cmake.in](../Dependencies.cmake.in)

```
if(NOT TARGET JsonCpp::JsonCpp)
  find_package(jsoncpp REQUIRED)
endif()
```


3. Update CMakeLists.txt accordingly with the new dependency

```
# Link
target_link_libraries(onbings-vista-objects
  PUBLIC

  PRIVATE
    JsonCpp::JsonCpp
)
```

4. Add the dependency to [vcpkg/port/vcpkg.json](../vcpkg/port/vcpkg.json) if it needs to be propagated to your dependees

```json
"dependencies": [
    { "name": "jsoncpp" }
  ],
```

## Removing

Simply executes the steps above in the reverse order.

# GIT submodule

This scenario is useful if you plan to develop both your code and the dependency at the same time

## Adding

Let's suppose we want to integrate EVS::evs-hwfw-logging library

1. Add the submodule to code/externals

```console
$> cd code/externals
$> git submodule add ssh://git@bitbucket.evs.tv:7999/evs-hwfw-libs/evs-hwfw-logging.git
```

2. Add the required [add_subdirectory](https://cmake.org/cmake/help/latest/command/add_subdirectory.html) to [code/CMakeLists.txt](../code/CMakeLists.txt) and potentially remove unwanted artifacts

```
set(EVS_HWFW_LOGGING_BUILD_TESTS OFF)
add_subdirectory(externals/evs-hwfw-logging)
```

3. Update CMakeLists.txt accordingly with the new dependency

```
# Link
target_link_libraries(onbings-vista-objects
  PUBLIC

  PRIVATE
    EVS::evs-hwfw-logging
)
```

4. **Add dependencies of the newly introduced dependency** (through vcpkg, submodule or both)

```json
"dependencies": [
    { "name": "spdlog" }
  ],
```

5. Add the dependency to [vcpkg/port/vcpkg.json](../vcpkg/port/vcpkg.json) if it needs to be propagated to your dependees

```json
"dependencies": [
    { "name": "evs-hwfw-logging" }
  ],
```

### Caveats

1. Point 5 above is only effective if you release both libraries synchronously
2. Cloning the project is no longer enough as externals are not retrieved by default.

```console
$> git clone ssh://git@bitbucket.evs.tv:7999/evs-hw/onbings-vista.git
$> git submodule update --init --recursive
```

## Removing

1. Undo steps 5 to 2 above
2. Remove the submodule

```console
$> git rm code/externals/evs-hwfw-logging.git
```

As the docs note however, the .git dir of the submodule is kept around (in the modules/ directory of the main project's .git dir), "to make it possible to checkout past commits without requiring fetching from another repository".
If you nonetheless want to remove this info, manually delete the submodule's directory in .git/modules/, and remove the submodule's entry in the file .git/config.

These steps can be automated using the commands

```
rm -rf .git/modules/code/externals/evs-hwfw-logging, and
git config --remove-section submodule.code/externals/evs-hwfw-logging
```

Credits : https://stackoverflow.com/questions/1260748/how-do-i-remove-a-submodule
