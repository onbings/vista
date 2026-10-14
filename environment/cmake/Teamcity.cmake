# We are running under teamcity
if(DEFINED ENV{TEAMCITY_VERSION})
  message("##teamcity[buildNumber '${CMAKE_PROJECT_VERSION} (${SOURCE_CONTROL_COMMIT_SHORT})']")
endif()
