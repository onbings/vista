include(CMakeDependentOption)

find_package(Python COMPONENTS Interpreter)

cmake_dependent_option(
  ENABLE_BUILD_PYTHON_WHEEL
  "Allow building python wheel for python module"
  ON
  "Python_Interpreter_FOUND"
  OFF
)

if(ENABLE_BUILD_PYTHON_WHEEL)
  include(${CMAKE_CURRENT_LIST_DIR}/python/build_wheel.cmake)
endif()