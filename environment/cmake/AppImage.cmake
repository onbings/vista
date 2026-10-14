include(CMakeParseArguments)

find_program(LINUXDEPLOY_COMMAND
  NAMES 
    linuxdeploy
    linuxdeploy-x86_64.AppImage
)

# linuxdeploy not found
if(NOT LINUXDEPLOY_COMMAND)
  return()
endif()

# Get the version
execute_process(
  COMMAND ${LINUXDEPLOY_COMMAND} --version
  OUTPUT_VARIABLE  LINUXDEPLOY_VERSION
  ERROR_VARIABLE   LINUXDEPLOY_VERSION 
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE
)

#
# Description
#   This function generates an AppImage
#
#   generate_appimage(
#     TARGET         <target>
#     [ICON          <file>]
#     [DESKTOP_FILE  <file>]
#     [CUSTOM_APPRUN <file>]
#     [COPY_FILE     <file_1> <rel_path_1>]
#       [COPY_FILE     <file_2> <rel_path_2>]
#     [COPY_DIR      <dir_1>  <rel_path_1>]
#       [COPY_DIR      <dir_2>  <rel_path_2>]
#     [OUTPUT_DIR    <dir>]
#     [EXCLUDE_SYSROOT]
#   )
#
# Parameter
#  TARGET           - The CMake target of the application
#  ICON             - An icon 
#  DESKTOP_FILE     - A desktop file according desktop entry specification : https://specifications.freedesktop.org/desktop-entry-spec/latest/
#  COPY_FILE        - A directive to copy a file inside the appimage. The destination is relative to {AppDir}/usr/
#  COPY_DIR         - A directive to copy a directory inside the appimage. The destination is relative to {AppDir}/usr/
#  CUSTOM_APPRUN    - A custom file to execute upon launching the appimage (e.g. startup script)
#  OUTPUT_DIR       - The directory path where to output the resulting appimage
#  EXCLUDE_SYSROOT  - A flag to exclude any library in a potential sysroot directory (used only if CMAKE_SYSROOT is defined)
#
function(generate_appimage)

  message(
    DEPRECATION
    "The function generate_appimage is deprecated. Please use evs_hwfw_make_appimage from the evs-hwfw-cmake package instead"
  )

  set(flags          EXCLUDE_SYSROOT)
  set(oneValueArgs   TARGET ICON DESKTOP_FILE CUSTOM_APPRUN OUTPUT_DIR)
  set(multiValueArgs COPY_FILE COPY_DIR)

  cmake_parse_arguments(arg "${flags}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(arg_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown parameters ${arg_UNPARSED_ARGUMENTS} for generate_appimage")
  endif()

  if(NOT arg_TARGET)
    message(FATAL_ERROR "Parameter TARGET is mandatory for generate_appimage. Please specify it")
  endif()

  set(TARGET_NAME  ${arg_TARGET})
  
  # If a desktop file is provide, take it
  # Otherwise, ask the tool to create one
  if(arg_DESKTOP_FILE)
    set(DESKTOP_FILE_OPTION "--desktop-file=\"${arg_DESKTOP_FILE}\"")
  else()
    set(DESKTOP_FILE_OPTION "--create-desktop-file")
  endif()

  # If an icon is provided, take it.
  # Otherwise, create an empty svg
  if(arg_ICON)
    set(ICON_FILE ${arg_ICON})
  else()
    file(TOUCH     ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.svg)
    set (ICON_FILE ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.svg)
  endif()

  if(arg_CUSTOM_APPRUN)
    set(CUSTOM_APPRUN "--custom-apprun=${arg_CUSTOM_APPRUN}")
  endif()

  if(arg_OUTPUT_DIR)
    set(OUTPUT_DIR ${arg_OUTPUT_DIR})
  else()
    set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/appimages)
  endif()

  # Make sure output directory exists
  set(APPDIR            ${CMAKE_CURRENT_BINARY_DIR}/AppDir)
  set(TARGET_FILE_NAME  ${TARGET_NAME}-${CMAKE_SYSTEM_PROCESSOR}.AppImage)

  file(MAKE_DIRECTORY ${OUTPUT_DIR})
  file(MAKE_DIRECTORY ${APPDIR})

  # ================
  # == COPY FILES ==
  # ================

  list(LENGTH arg_COPY_FILE nb_COPY_FILE)

  while(nb_COPY_FILE GREATER 0)
    list(POP_FRONT arg_COPY_FILE file dest)

    if(NOT dest)
      message(FATAL_ERROR "Could not copy file ${file} as no destination was provided")
    endif()

    list(APPEND ADDITIONAL_FILES ${file})

    list(APPEND COPY_FILES COMMAND ${CMAKE_COMMAND} -E make_directory ${APPDIR}/usr/${dest}/)  
    list(APPEND COPY_FILES COMMAND ${CMAKE_COMMAND} -E copy  ${file}  ${APPDIR}/usr/${dest}/)
    list(LENGTH arg_COPY_FILE nb_COPY_FILE)

  endwhile()

  # ====================
  # == COPY DIRECTORY ==
  # ====================

  list(LENGTH arg_COPY_DIR nb_COPY_DIR)

  while(nb_COPY_DIR GREATER 0)
    list(POP_FRONT arg_COPY_DIR dir dest)

    # Make sure source file exists
    if(NOT EXISTS ${dir})
      message(FATAL_ERROR "Could not copy directory ${dir} as it does not exist")
    endif()

    if(NOT dest)
      message(FATAL_ERROR "Could not copy directory ${dir} as no destination was provided")
    endif()

    list(APPEND ADDITIONAL_FILES ${dir})

    list(APPEND COPY_DIRS COMMAND ${CMAKE_COMMAND} -E copy_directory ${dir} ${APPDIR}/usr/${dest})
    list(LENGTH arg_COPY_DIR nb_COPY_DIR)

  endwhile()

  configure_file(
    ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/linuxdeploy.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/linuxdeploy.sh
    @ONLY
    FILE_PERMISSIONS 
      OWNER_READ
      OWNER_EXECUTE
      GROUP_READ
      GROUP_EXECUTE
  )

  #
  # If we are cross-compiling, exclude all the libraries
  # from the sysroot as we know those libraries will
  # already be available on the target system
  #
  set(EXCLUSION_LIST "")

  if(arg_EXCLUDE_SYSROOT AND DEFINED CMAKE_SYSROOT)
    file(GLOB_RECURSE SYSROOT_LIBS RELATIVE "${CMAKE_SYSROOT}/lib" "${CMAKE_SYSROOT}/lib/*.so*")

    foreach(SYSROOT_LIB ${SYSROOT_LIBS})
      list(APPEND EXCLUSION_LIST "--exclude-library=${SYSROOT_LIB}")
    endforeach()
  endif()

  add_custom_command(
    OUTPUT  ${OUTPUT_DIR}/${TARGET_FILE_NAME}
    COMMENT "Generating AppImage for $<TARGET_FILE:${TARGET_NAME}>"
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${APPDIR}
    ${COPY_FILES}
    ${COPY_DIRS}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/linuxdeploy.sh 
                --appdir="${APPDIR}"
                ${EXCLUSION_LIST}
                --executable="$<TARGET_FILE:${TARGET_NAME}>"
                ${DESKTOP_FILE_OPTION}
                --icon-file="${ICON_FILE}"
                --output=appimage
                ${CUSTOM_APPRUN}
    DEPENDS ${TARGET_NAME}
            ${ADDITIONAL_FILES}
            ${arg_ICON}
            ${arg_DESKTOP_FILE}
            ${arg_CUSTOM_APPRUN}
    WORKING_DIRECTORY ${OUTPUT_DIR}
  )

  add_custom_target(${TARGET_NAME}-appimage
    ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Building AppImage"
    DEPENDS ${OUTPUT_DIR}/${TARGET_FILE_NAME}
  )

endfunction()
