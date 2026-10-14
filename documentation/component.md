# Introduction

It's common in a repository to have more than one component

## Adding

The convenience script [add-component.py](../add-component.py) allows you easily create a new component

### Usage

```console
$>./add-component.py [--lib] [--app] new-component [--public-deps "dep1,dep2"] [--private-deps "dep3,dep4"]
```

Will create your new component and add it automatically to your project.

If it's a library, it will create the library and the associated test application.
If it's an application, it will create it and add it to the list of sources to ignore for coverage

Besides --lib or --app, all other parameters are optional.

### Example

```console
$>./add-component.py --lib evs-hwfw-foo --private-deps EVS::evs-hwfw-bar
```
Will create a library **evs-hwfw-foo** which has a private dependency on **EVS::evs-hwfw-bar**

## Removal

1. Remove the component directory in [code/sources](../code/sources)
2. Remove the [add_subdirectory](https://cmake.org/cmake/help/latest/command/add_subdirectory.html) of the component in the [code/CMakeLists.txt](../code/CMakeLists.txt)
3. Optionnaly remove it from [coverage.exclusion](../coverage.exclusion) if it was an application
