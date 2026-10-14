# In practice

## Get vcpkg
```console
$>git clone ssh://git@bitbucket.evs.tv:7999/vcpkg/vcpkg.git
$>export VCPKG_ROOT=/path/to/vcpkg
```

## Get the sources
```console
$>export VCPKG_ROOT=/path/to/vcpkg
$>git clone ssh://git@bitbucket...
$>cd ...
$onbings-vista>git submodule update --init --recursive
```

## Lauching environment
```console
$>cd ./environment
$environment>.\env_launch.py -n xts
```

## Listing presets
```console
$>cd /tmp/build
$build>cmake -B . /path/to/onbings-vista --list-presets
```

## Configure using presets
```console
$>cd /tmp/build
$build>cmake -B . /path/to/onbings-vista --preset all-pc-linux
```

## Build
```console
$>cd /tmp/build
$build>cmake --build .
```

## Run tests
```console
$>cd /tmp/build
$build>ctest .
```

# Build environment

See [enviromnent](https://bitbucket.evs.tv/projects/VSE/repos/evs-hwfw-environment/browse) for more information

# Dependencies

To get a list of dependencies, check [vcpkg.json](../vcpkg.json)

To add or remove a dependency, refer to [dependencies](dependencies.md)

# CMake

In order to ease CMake argument passing, this project uses CMake presets.

CMake presets come with 2 files with different purposes

* [CMakePresets.json](../CMakePresets.json) : This file is git tracked and contains project-wide settings (i.e. usable by everyone)
* [CMakeUserPresets.json](../CMakeUserPresets.json) : This file is ignored by git and contains your personal user-defined settings if you need some (e.g. user local path)

It's safe to create and modify [CMakeUserPresets.json](../CMakeUserPresets.json) for your own purpose. [CMakePresets.json](../CMakePresets.json) should only be modified to add information relevant for everyone

Example of CMakeUserPresets.json could look like that

```
{
  "version": 4,
  "cmakeMinimumRequired": {
    "major" : 3,
    "minor" : 23,
    "patch" : 0
  },
  "configurePresets" : [
    {
      "name": "user-default",
      "hidden": true,
      "binaryDir": "/home/user/build/my-binary-dir",
      "toolchainFile" : "/home/user/git/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake"
    },
    {
      "name": "my-config",
      "hidden": false,
      "inherits" : [ "user-default", "all-pc-windows" ]
    }
  ]
}
```

which can be used like this

```console
$>cmake /path/to/project --preset my-config
```

Presets can be listed through

```console
$>cmake /path/to/project --list-presets
```
