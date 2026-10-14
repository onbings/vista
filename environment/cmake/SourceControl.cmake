
# The information was given by the user
if(DEFINED SOURCE_CONTROL_BRANCH AND DEFINED SOURCE_CONTROL_COMMIT_FULL)

  set(SOURCE_CONTROL_TYPE "User")

  # Get only the meaningfull part for the branch
  string(REPLACE "refs/heads/" "" SOURCE_CONTROL_BRANCH "${SOURCE_CONTROL_BRANCH}")

  # Get the length of the full commit revision
  string(LENGTH "${SOURCE_CONTROL_COMMIT_FULL}" FULL_LENGTH)

  # Infer short commit revision
  if(${FULL_LENGTH} GREATER 7)
    string(SUBSTRING "${SOURCE_CONTROL_COMMIT_FULL}" 0 7 SOURCE_CONTROL_COMMIT_SHORT)
  else()
    set(SOURCE_CONTROL_COMMIT_SHORT "${SOURCE_CONTROL_COMMIT_FULL}")
  endif()

# Looks like it's a git repository
elseif(EXISTS ${CMAKE_SOURCE_DIR}/.git)
  include(${CMAKE_CURRENT_LIST_DIR}/Git.cmake)
endif()

