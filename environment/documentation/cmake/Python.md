# Python.cmake

This module offers facilities to create Python package from CMake

## build_wheel

This feature is only available if a valid Python Interpreter is detected on the system

### CMake variables

| CMake variable            | Description
| ------------------------- | -----------
| ENABLE_BUILD_PYTHON_WHEEL | A boolean flag indicating if the build_wheel CMake function is available

This variable can be used to initialize dependent option values

```
cmake_dependent_option(MY_PROJECT_BUILD_PYTHON_WHEEL "Build Python wheel for my project" ON "ENABLE_BUILD_PYTHON_WHEEL" OFF)
```

### CMake function

When enabled, the following function is available

```
build_wheel(
  TARGET                <target>
  PYTHON_MODULE_NAME    <module_name>
  [WHEEL_PYTHON_TAG]    <python_tag>
  [WHEEL_ABI_TAG]       <abi_tag>
  [WHEEL_PLATFORM_TAG]  <platform_tag>
  [INIT_PY_PATH]        <path>
  [ADDITIONAL_FILES]    <file_1> .. <file_N>
  [OUTPUT_DIR]          <dir>
  [EXEC_SCRIPT]         <rule_1> .. <rule_N>
  [GUI_EXEC_SCRIPT]     <rule_1> .. <rule_N>
  [WHEEL_DEPENDENCIES]  <expression_1> .. <expression_N>
  [DEPENDS]             <dependency_1> .. <dependency_N>
)
```

This function produces a file python wheel packaged named <module_name>-<version>-<python_tag>-<abi_tag>-<platform_tag>.whl in ${CMAKE_BINARY_DIR}/wheels

| Parameter           | Description
| ------------------- | -----------
| TARGET              | The CMake target of the python module 
| PYTHON_MODULE_NAME  | The name of the python module
| WHEEL_PYTHON_TAG    | The python tag of the created wheel (e.g. cp312 for python 3.12)
| WHEEL_ABI_TAG       | The ABI tag of the created wheel (e.g. abi3 for stable ABI of python3)
| WHEEL_PLATFORM_TAG  | The platform tag of the created wheel (e.g. linux_x64_86)
| INIT_PY_PATH        | The path to a __init__.py to use for the module
| ADDITIONAL_FILES    | Additional files to include in the package
| OUTPUT_DIR          | The directory where to output the wheel
| EXEC_SCRIPT         | A list of expressions to declare executables from scripts installed by your package (see EXEC_SCRIPT section below for syntax)
| GUI_EXEC_SCRIPT     | The same as above for GUI application (i.e. not starting a shell)
| WHEEL_DEPENDENCIES  | A list of dependencies expression for your wheel (see WHEEL_DEPENDENCIES section below)
| DEPENDS             | A list of additional files or targets that once modified should retrigger building the wheel

**Remarks**

* If not provided additional arguments are best guessed based on CMake variables

### EXEC_SCRIPT and GUI_EXEC_SCRIPT

In order to register application from scripts installed by your wheel. You need to register them using the following syntax

**<program_name>=<module_name>:<function>**

#### Example 

```
build_wheel(
  ...
  EXEC_SCRIPT my-super-program=my_module:main
)
```

#### Details

Suppose you have the following python script (hello_world.py) which is executable

```python
def main():
    print ("Hello world")
 
if __name__ == "__main__":
    try :
        sys.exit(main())
    except Exception as e :
        print(e)
        sys.exit(1)
```

You need to register it from the pyproject.toml to have an executable called "hello-world"

```pyproject.toml
[project]
  name = "hello_world"
[project.scripts]
  hello-world="hello_world:main"
```

### WHEEL_DEPENDENCIES

As explained in [PEP-0621](https://peps.python.org/pep-0621/#dependencies-optional-dependencies), you can express additional package dependencies that will be installed with your wheel.

Check [PEP-0508](https://peps.python.org/pep-0508/) for valid syntax 

#### Example

```
  set(WHEEL_DEPENDENCIES 
    "PyQt5==5.15.11"
    "evs_hwfw_hardware_setup==${PROJECT_VERSION}"
  )
  
  build_wheel(
    ...
    WHEEL_DEPENDENCIES ${WHEEL_DEPENDENCIES}
  )
  
```

### Examples

#### Simple binary

```
nanobind_add_module(my-target
  STABLE_ABI
  NB_STATIC
  src/python-bindings.cpp
)

if(MY_PROJECT_BUILD_PYTHON_PACKAGE)
  build_wheel(
    TARGET              my-target
    PYTHON_MODULE_NAME "my_module"
  )
endif()
```

Possible output : my_module-cp312-abi3-linux_x64_86.whl

#### Packaging a "pure" python script

```
#
# Create a "dummy" target to make the file "required"
#
add_library(evs-hwfw-hardware-setup-gui INTERFACE
  src/evs-hwfw-hardware-setup-gui.py
  src/README.md
  src/requirements.txt
)

#
# Create a python to install the script
#
if(EVS_HWFW_HARDWARE_SETUP_BUILD_WHEEL)

  set(WHEEL_DEPENDENCIES 
    "PyQt5==5.15.11"
    "evs_hwfw_hardware_setup==${PROJECT_VERSION}"
  )

  build_wheel(
    TARGET              evs-hwfw-hardware-setup-gui
    INIT_PY_PATH        ${CMAKE_CURRENT_SOURCE_DIR}/src/evs-hwfw-hardware-setup-gui.py
    PYTHON_MODULE_NAME  evs_hwfw_hardware_setup_gui
    WHEEL_DEPENDENCIES  ${WHEEL_DEPENDENCIES}
    WHEEL_PYTHON_TAG    py${Python_VERSION_MAJOR}
    WHEEL_ABI_TAG       none
    WHEEL_PLATFORM_TAG  any
    EXEC_SCRIPT         "evs-hwfw-hardware-setup-gui=\"evs_hwfw_hardware_setup_gui:main\""
  )

endif()
```

Possible output : evs_hwfw_hardware_setup_gui-0.0.0.1-py3-none-any.whl