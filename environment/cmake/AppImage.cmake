
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
