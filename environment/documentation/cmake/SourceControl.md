# SourceControl.cmake

This module is responsible for detecting the source control (if any) and reporting associated information

## CMake variables

| Name                        | Description                          | Example
| --------------------------- | -------------                        | ----------- 
| SOURCE_CONTROL_TYPE         | The type of source control           | Git
| SOURCE_CONTROL_BRANCH       | The repository branch                | develop
| SOURCE_CONTROL_COMMIT_FULL  | The repository full commit revision  | 790cc40f9c92c13e79a9d1fc43addce5fc175ec4
| SOURCE_CONTROL_COMMIT_SHORT | The repository short commit revision | 790cc40
| SOURCE_CONTROL_URL          | The URL of the repository            | ssh://git@bitbucket.evs.tv:7999/evs-hw-r6x/evs-hwfw-r6x.git

**Remark**

If those variables are given externally (e.g. by a CI system), this module won't overwrite them