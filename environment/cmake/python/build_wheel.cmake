function(build_wheel)

  message(
    DEPRECATION
    "The function build_wheel is deprecated. Please use evs_hwfw_build_wheel from the evs-hwfw-cmake package instead"
  )

  set(flags "")
  set(oneValueArgs
    TARGET              # The name of the python binding target
    PYTHON_MODULE_NAME  # The name of the python module to be created
    WHEEL_PYTHON_TAG    # The python tag for the wheel                      (optional)
    WHEEL_ABI_TAG       # The abi tag for the wheel                         (optional)
    WHEEL_PLATFORM_TAG  # The platform tag for the wheel                    (optional)
    INIT_PY_PATH        # The path to the __init__.py file                  (optional)
    OUTPUT_DIR          # The output directory for the wheel                (optional)
  )
  set(multiValueArgs
    ADDITIONAL_FILES    # Additional files to be copied to the project directory (optional)
    WHEEL_DEPENDENCIES  # A list of dependencies of the wheel (optional)
    DEPENDS             # A list of files that should retrigger building the wheel if modified (optional)
    EXEC_SCRIPT         # The entry point(s) of the executable(s) to register     (optional)
    GUI_EXEC_SCRIPT     # The entry point(s) of the executable(s) GUI to register (optional)
  )

  cmake_parse_arguments(arg "${flags}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown parameters ${arg_UNPARSED_ARGUMENTS} for build_wheel")
  endif()

  if(NOT arg_TARGET)
    message(FATAL_ERROR "Parameter TARGET is mandatory for build_wheel. Please specify it")
  endif()

  if(NOT arg_PYTHON_MODULE_NAME)
    message(FATAL_ERROR "Parameter PYTHON_MODULE_NAME is mandatory for build_wheel. Please specify it")
  endif()

  #
  # Setup variables
  #
  set(TARGET              ${arg_TARGET}) 
  set(PYTHON_MODULE_NAME  ${arg_PYTHON_MODULE_NAME})
  set(VENV_DIR            ${CMAKE_BINARY_DIR}/python_venv)
  set(DEPENDENCIES        ${arg_DEPENDS} ${arg_INIT_PY_PATH} ${arg_ADDITIONAL_FILES})

  get_target_property(TARGET_TYPE ${TARGET} TYPE)

  if(${TARGET_TYPE} STREQUAL "INTERFACE_LIBRARY")
    set(IS_PURE_PYTHON ON)

    if(NOT arg_INIT_PY_PATH)
      message(FATAL_ERROR "Parameter INIT_PY_PATH is mandatory for pure python packages")
    endif()

  else()
    set(IS_PURE_PYTHON OFF)
  endif()

  string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} CMAKE_SYSTEM_PROCESSOR_LOWER)

  if(WIN32)
    set(PLATFORM_TAG_PREFIX "win")
    set(VENV_BIN ${VENV_DIR}/Scripts)
  else()
    set(PLATFORM_TAG_PREFIX "linux")
    set(VENV_BIN ${VENV_DIR}/bin)
  endif()

  if(IS_PURE_PYTHON)
    set(WHEEL_PYTHON_TAG   py${Python_VERSION_MAJOR})
    set(WHEEL_PLATFORM_TAG any)
    set(WHEEL_ABI_TAG      none)
  else()
    set(WHEEL_PYTHON_TAG    cp${Python_VERSION_MAJOR}${Python_VERSION_MINOR})
    set(WHEEL_PLATFORM_TAG  ${PLATFORM_TAG_PREFIX}_${CMAKE_SYSTEM_PROCESSOR_LOWER})

    # This is ugly but it when using nanobind it's the only
    # way to know if we are using the stable ABI or not
    if(NOT TARGET nanobind-static-abi${Python_VERSION_MAJOR})
      set(WHEEL_ABI_TAG "${WHEEL_PYTHON_TAG}")
    else()
      set(WHEEL_ABI_TAG "abi${Python_VERSION_MAJOR}")
    endif()
  endif()

  #
  # Override default arguments with provided ones if any
  #
  if(arg_WHEEL_PYTHON_TAG)
    set(WHEEL_PYTHON_TAG ${arg_WHEEL_PYTHON_TAG})
  endif()

  if(arg_WHEEL_ABI_TAG)
    set(WHEEL_ABI_TAG ${arg_WHEEL_ABI_TAG})
  endif()

  if(arg_WHEEL_PLATFORM_TAG)
    set(WHEEL_PLATFORM_TAG ${arg_WHEEL_PLATFORM_TAG})
  endif()

  #
  # Create the directory tree to package the python module
  #
  set (SRC_PACKAGING_DIR  ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
  set (DST_PACKAGING_DIR  ${CMAKE_CURRENT_BINARY_DIR}/packaging/${PYTHON_MODULE_NAME})
  set (DST_PROJECT_DIR    ${DST_PACKAGING_DIR}/${PYTHON_MODULE_NAME})

  if(arg_OUTPUT_DIR)
    set (DST_WHEELS_DIR   ${arg_OUTPUT_DIR})
  else()
    set (DST_WHEELS_DIR   ${CMAKE_BINARY_DIR}/wheels)
  endif()
   
  file(MAKE_DIRECTORY     ${DST_PACKAGING_DIR})
  file(MAKE_DIRECTORY     ${DST_PROJECT_DIR})
  file(MAKE_DIRECTORY     ${DST_WHEELS_DIR})

  # Collect optional dependencies
  list(TRANSFORM arg_WHEEL_DEPENDENCIES PREPEND "\"")
  list(TRANSFORM arg_WHEEL_DEPENDENCIES APPEND  "\"")
  list(JOIN      arg_WHEEL_DEPENDENCIES ",\n\t" PYTHON_MODULE_DEPENDENCIES)

  # Setup executable to register (if any)
  if(arg_EXEC_SCRIPT)
    string(APPEND PYTHON_MODULE_EXECUTABLE_SECTION "[project.scripts]\n")
    
    foreach(entry ${arg_EXEC_SCRIPT})
      string(APPEND PYTHON_MODULE_EXECUTABLE_SECTION "\t${entry}")
    endforeach()
    
    string(APPEND PYTHON_MODULE_EXECUTABLE_SECTION "\n")
  endif()
  
  if(arg_GUI_EXEC_SCRIPT)
    string(APPEND PYTHON_MODULE_EXECUTABLE_SECTION "[project.gui-scripts]\n")
    
    foreach(entry ${arg_GUI_EXEC_SCRIPT})
      string(APPEND PYTHON_MODULE_EXECUTABLE_SECTION "\t${entry}")
    endforeach()
    
    string(APPEND PYTHON_MODULE_EXECUTABLE_SECTION "\n")
  endif()

  # Create the toml file
  configure_file(
    ${SRC_PACKAGING_DIR}/pyproject.toml.in
    ${DST_PACKAGING_DIR}/pyproject.toml
    @ONLY
  )

  # Provide the __init__.py file
  if(arg_INIT_PY_PATH)
    configure_file(
      ${arg_INIT_PY_PATH}
      ${DST_PROJECT_DIR}/__init__.py
      COPYONLY
    )
  else()
    configure_file(
      ${SRC_PACKAGING_DIR}/__init__.py.in
      ${DST_PROJECT_DIR}/__init__.py
      @ONLY
    )
  endif()

  # Add additional files if any
  if(arg_ADDITIONAL_FILES)
    foreach(src ${arg_ADDITIONAL_FILES})
      cmake_path(GET src FILENAME file_name)
      set       (dst ${DST_PROJECT_DIR}/${file_name})
      list      (APPEND DEPENDENCIES ${dst})      

      add_custom_command(
        OUTPUT  ${dst}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${src} ${dst}
        DEPENDS ${src}
      )

    endforeach()
  endif()

  #
  # Setup a python virtualenv
  #
  if(NOT EXISTS ${VENV_DIR})
    set(CREATE_VENV ON)
  else()
    set(CREATE_VENV OFF)
  endif()

  if(CREATE_VENV)
    message(STATUS "Creating python virtualenv in ${VENV_DIR}")
    execute_process(
      COMMAND ${Python_EXECUTABLE} -m venv ${VENV_DIR}
      COMMAND_ERROR_IS_FATAL ANY
    )
  endif()

  # Simulate the behavior of activating the virtualenv
  set         (ENV{VIRTUAL_ENV} ${VENV_DIR})
  set         (Python_FIND_VIRTUALENV ONLY)
  unset       (Python_EXECUTABLE)
  find_package(Python REQUIRED COMPONENTS Interpreter)

  # Here we are properly pointing the python executable in the venv
  if(CREATE_VENV)
    execute_process(COMMAND ${Python_EXECUTABLE} -m pip install --upgrade pip                     COMMAND_ERROR_IS_FATAL ANY)
    execute_process(COMMAND ${Python_EXECUTABLE} -m pip install auditwheel build setuptools wheel COMMAND_ERROR_IS_FATAL ANY)
  endif()

  set(INTERMEDIATE_WHEEL ${DST_PACKAGING_DIR}/dist/${PYTHON_MODULE_NAME}-${PROJECT_VERSION}-${WHEEL_PYTHON_TAG}-${WHEEL_ABI_TAG}-${WHEEL_PLATFORM_TAG}.whl)

  # Once the wheel is created it can be audited
  # to see if it can claim a "lower requirements"
  # tag such as "manylinux_X_Y" (only under linux) 
  if(DEFINED CMAKE_SYSROOT OR (NOT LINUX) OR IS_PURE_PYTHON)
    # Audit wheel is not cross-platform so simply copy the wheel
    # to the target directory when cross-compiling
    set(AUDIT_WHEEL_CMD
      ${CMAKE_COMMAND} -E copy_if_different
        ${INTERMEDIATE_WHEEL}
        ${DST_WHEELS_DIR}
    )
  else()
    # Retrieve the version of GNU libc
    execute_process(
      COMMAND ldd --version
      OUTPUT_VARIABLE LDD_VERSION
      OUTPUT_STRIP_TRAILING_WHITESPACE
      COMMAND_ERROR_IS_FATAL ANY
    )

    # Convert lines to a list
    string  (REPLACE "\n" ";" LDD_VERSION_LINES ${LDD_VERSION})
    # Pick-up the first line (e.g. ldd (GNU libc) 2.34)
    list    (POP_FRONT LDD_VERSION_LINES FIRST_LINE)
    # Tokenize the line
    string  (REPLACE  " " ";" FIRST_LINE_LIST ${FIRST_LINE})
    # Pick up the last token (should be the version)
    list    (POP_BACK FIRST_LINE_LIST GNU_LIBC_VERSION)
    # Replace the dot with an underscore
    string  (REPLACE "." "_" GNU_LIBC_VERSION ${GNU_LIBC_VERSION})

    set(AUDIT_WHEEL_CMD
      ${VENV_BIN}/auditwheel repair
        --plat manylinux_${GNU_LIBC_VERSION}_${CMAKE_SYSTEM_PROCESSOR_LOWER}
        --wheel-dir ${DST_WHEELS_DIR}
        ${INTERMEDIATE_WHEEL}
    )
  endif()

  # Prepare common commands

  set(CLEAN_BUILD_DIR_COMMANDS
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${DST_PACKAGING_DIR}/build
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${DST_PACKAGING_DIR}/dist
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${DST_PACKAGING_DIR}/${PYTHON_MODULE_NAME}.egg-info
  )

  set(BUILD_WHEEL_COMMANDS
    # Build the wheel  
    COMMAND ${Python_EXECUTABLE} -m build --wheel
    # Re-tag the wheel properly
    COMMAND ${VENV_BIN}/wheel tags 
      --python-tag   ${WHEEL_PYTHON_TAG}
      --abi-tag      ${WHEEL_ABI_TAG}
      --platform-tag ${WHEEL_PLATFORM_TAG}
      --remove
      ${DST_PACKAGING_DIR}/dist/${PYTHON_MODULE_NAME}-${PROJECT_VERSION}-py3-none-any.whl
    # Copy the wheel to the base wheels directory (and optionally repair it)
    COMMAND ${AUDIT_WHEEL_CMD}
  )

  #
  # If the given target is interface library, it does not compile
  # (and hence does not produce any output), so we cannot use
  # custom command with post build steps. Instead, we specify
  # the output it will generate and create a "dummy" interface
  # that depends on it to make sure the command is called whenever
  # something changes.
  #
  if(IS_PURE_PYTHON)
  
    # Create the wheel
    add_custom_command(
      OUTPUT  ${INTERMEDIATE_WHEEL}
      # Clean the build directory
      ${CLEAN_BUILD_DIR_COMMANDS}
      # Build the wheel
      ${BUILD_WHEEL_COMMANDS}
      DEPENDS ${DEPENDENCIES}
      WORKING_DIRECTORY ${DST_PACKAGING_DIR}
    )
  
  else()
  
    # Strip the target if possible
    if(CMAKE_STRIP)
      set(STRIP_OR_COPY_COMMAND 
          COMMAND ${CMAKE_STRIP}
          -s 
          -o ${DST_PACKAGING_DIR}/${PYTHON_MODULE_NAME}/$<TARGET_FILE_NAME:${TARGET}>
          $<TARGET_FILE:${TARGET}>  
      )
    else()
      set(STRIP_OR_COPY_COMMAND 
          COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            $<TARGET_FILE:${TARGET}>
            ${DST_PACKAGING_DIR}/${PYTHON_MODULE_NAME}/
      )
    endif()

    list(APPEND DEPENDENCIES ${TARGET})

    #
    # This is a target that builds something
    # so copy that something into the module
    #
    add_custom_command(
      OUTPUT  ${INTERMEDIATE_WHEEL}
      # Clean the build directory
      ${CLEAN_BUILD_DIR_COMMANDS}
      # Copy the python module
      ${STRIP_OR_COPY_COMMAND}
      # Build the wheel  
      ${BUILD_WHEEL_COMMANDS}
      DEPENDS ${DEPENDENCIES}
      WORKING_DIRECTORY ${DST_PACKAGING_DIR}
    )

  endif()

  # Attach it to a dummy target so that the "build wheel"
  # commands are called when something changes
  file(WRITE ${DST_PACKAGING_DIR}/dummy.cpp)

  add_library(${TARGET}-wheel INTERFACE
    ${INTERMEDIATE_WHEEL}
    ${DST_PACKAGING_DIR}/dummy.cpp
    ${arg_ADDITIONAL_FILES}
  )

endfunction()