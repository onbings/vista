# Introduction

This repository contains all the facilities required to build and develop XTS components.

This includes
* The proper docker image(s) that contains required compilers and tools (GCC, CMake, Ninja, gcov, ...)
* The optional CMake scripts to enable and automate extra tasks (Code coverage, Memory checking, additional compile flags, ...)
* The vcpkg triplets for the supported platforms

## Getting Started

### Get the sources

```console
$ git clone ssh://git@bitbucket.evs.tv:7999/vse/evs-hwfw-environment.git
```

### Determining supported environment

```console
$evs-hwfw-environment>.\env_launch --help
...
Required arguments:
  -n NAME, --name NAME  The name of the environment to launch (found : ['xts'])
...
```

Check **found** values.

> Remark : You can only execute Linux environment on Linux Host and Windows environment on Windows host.

### Launching the environment

```console
$environment>.\env_launch -n xts
```

A hash is computed for the whole directory recursively to determine the "version" of the environment.

Then the following are performed in order to resolve the image to launch based on the given name

1. The local Docker cache is checked for an image matching <name:hash>
2. The remote Docker repository on Artifactory is checked for an image matching <name:hash>
3. The associated Dockerfile.<name> is rebuilt

### Modifying environment

All known environments are described in Dockerfile located in **.\dockers\Dockerfile.<environment>**

The Dockerfile is both the recipe to build the environment and the documentation about the requirements to build projects.

Once modified, simply rerunning the env_launch.py script should automatically rebuild and rerun the proper environment as any modification in this directory will generate a new hash.

### Environment caching

When given valid write credentials, the built Docker image will be cached on Artifactory (and thus available to anyone)

```console
$environment>.\env_launch -n xts --user <user> --password <password>
```

> Remark : This is typically done by Teamcity agents

## CMake modules

**Important remark**

The CMake modules defined here provides enhanced or additional functionalities not provided by default by CMake.

Consequently, they **should not** be a requirement to build your project.

### Enabling

Simply include main.cmake from your top CMakeLists.txt

```
# We are the root directory
if(("x${CMAKE_SOURCE_DIR}x" STREQUAL "x${CMAKE_CURRENT_SOURCE_DIR}x") AND
   (EXISTS ${CMAKE_SOURCE_DIR}/environment/cmake/main.cmake))
   include(${CMAKE_SOURCE_DIR}/environment/cmake/main.cmake)
endif()
```

### Module documentation

| Module                                                      | Description
| ----------------------------------------------------------- | ---------------------------------------------------------------------------------------------
| [Analysis.cmake](documentation/cmake/Analysis.md)           | Add support for code analysis tools such as "clang-tidy", "iwyu", etc, ...
| [AppImage.cmake](documentation/cmake/AppImage.md)           | Add support for AppImage generation
| [DetectToolset.cmake](documentation/cmake/DetectToolset.md) | Detect toolset and enforces compiler settings accordingly
| [Python.cmake](documentation/cmake/Python.md)               | Provides facilities to create Python packages
| [SourceControl.cmake](documentation/cmake/SourceControl.md) | Detect your source control (if any) and populate variables regarding branch, commit, etc, ...
| [Teamcity.cmake](documentation/cmake/Teamcity.md)           | Report version number to Teamcity
| [Testing.cmake](documentation/cmake/Testing.md)             | Provides facilities (coverage, memory checking, ...) for CTest
| [Vcpkg.cmake](documentation/cmake/Vcpkg.md)                 | Add support for vcpkg like an install target for vcpkg-dependencies, etc, ...
| [Versioning.cmake](documentation/cmake/Versioning.md)       | Update build number of version number

## Contacts

* **Bernard HARMEL** [b.harmel@evs.com](b.harmel@evs.com)

