# Teamcity.cmake

This module is responsible for reporting information to Teamcity.

## Version

It reports build number through [Teamcity service message](https://www.jetbrains.com/help/teamcity/service-messages.html#Service+Messages+Formats)

```
message("##teamcity[buildNumber '${CMAKE_PROJECT_VERSION} (${SOURCE_CONTROL_COMMIT_SHORT})']")
```
