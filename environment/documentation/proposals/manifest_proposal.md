# Introduction

When packaging a full software suite we want to be able to answer the following questions

1. Is my packaging consistent ?
2. What are all the components that where used to produce this package ?

In order to answer them, we need to keep track of build information and metadata in a file called a manifest.

## 1. Consistency

Given artifacts provide a manifest themselves, consistency can be checked.

See the section **Enforcing Contracts** of the [contract proposal](contract_proposal.md) for a concrete way to do it.

## 2. Auditing

Listing all the dependencies used to create all the artifacts a package contains is often an interesting piece of information to track.

Example :

- The license for library X changed, do I use it ?
- The library Y at version 1.2.3.4 is vulnerable, I am impacted ?

Keeping track of those information alongside the package allows to rapidly and consistently answer those questions.

# Manifest format

We need to distinguish 2 types of manifests
  - application manifest
  - package manifest.

## Application manifest

We propose to create the manifest in JSON with the following fields

 - **name** : The name of the application
 - **version** : The version of the application
 - **type** : Always **application**
 - **contracts** : The array representing the various [contracts](contract_proposal.md) of the application
 - **vcpkg_dependencies** : The array listing the dependencies retrieved from vcpkg
 
### Example

```json
{
  "name" : "application_name",
  "version" : "1.2.3.4",
  "type": "application",
  "contracts": [
    {
      "name": "evs-hwfw-hardware-setup",
      "version": "0.0.3.65535",
      "policy": "same_minor"
    },
    {
      "name": "evs-hwfw-board-maintenance",
      "version": "0.0.4.65535"
    }
  ],
  "vcpkg_dependencies": {
    "abseil:evs-x64-linux": {
      "package_name": "abseil",
      "triplet": "evs-x64-linux",
      "version": "20250127.0",
      "port_version": 0,
      "features": [],
      "desc": [
        "Abseil is an open-source collection of C++ library code designed to augment the C++ standard library. The Abseil library code is collected from Google's own C++ code base, has been extensively tested and used in production, and is the same code we depend on in our daily coding lives.",
        "In some cases, Abseil provides pieces missing from the C++ standard; in others, Abseil provides alternatives to the standard for special needs we've found through usage in the Google code base. We denote those cases clearly within the library code we provide you.",
        "Abseil is not meant to be a competitor to the standard library; we've just found that many of these utilities serve a purpose within our code base, and we now want to provide those resources to the C++ community as a whole."
      ]
    },
    ...
  }
}
```
### Building

Application manifests will be produced by CMake when building the application.

If needed, application manifest should be installed at [${CMAKE_INSTALL_DATADIR}](https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html)

## Package manifest

When a packager creates a package, it can simply "aggregates" all the manifests it found when packaging.

It will thus have the following fields

- **name** : The name of the package
- **version** : The version of the package
- **type** : Always **package**
- **manifests**: The array of packaged manifests

### Example

```json
{
  "name": "v6x-rootfs.tar.gz",
  "version": "1.2.3.4",
  "type": "package",
  "manifests": [
    {
      "name": "v6xd",
      "version": "1.1.1.1",
      "type": "application",
      "contracts" : [ ... ]
      "vcpkg_dependencies": []
    },
    {
      "name": "another_package",
      "version": "2.2.2.2",
      "type": "package",
      "manifests":[...]
    }
    ...
  ]
}
```

### Remarks

1. A package can contain other packages.

### Building

Package manifest will be produced when packaging, either by **Buildroot** or **evs-emotion-os** packager

Remark : Buildroot might already provide other useful information

# Conventions

## Naming

Given an artifact called **name**, we recommend naming the manifest **name.manifest.json**

Example : The manifest for application **evs-hwfw-proxy-xx86_64.AppImage** should be named **evs-hwfw-proxy-xx86_64.AppImage.manifest.json**

## Storage

Manifest should be stored next to the artifact.

```
|--directory
|  |---v6x-rootfs.tar.gz
|  |---v6x-rootfs.tar.gz.manifest.json
```
