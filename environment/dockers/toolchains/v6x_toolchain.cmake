set(CMAKE_SYSTEM_NAME      Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_SYSROOT          /opt/sdk/v6x/aarch64-evs-linux-gnu/sysroot)
set(CMAKE_C_COMPILER       /opt/sdk/v6x/bin/aarch64-evs-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER     /opt/sdk/v6x/bin/aarch64-evs-linux-gnu-g++)

SET(CMAKE_C_FLAGS_INIT   "-g -fPIC -mcpu=cortex-a53 -mfix-cortex-a53-835769 -mfix-cortex-a53-843419")
SET(CMAKE_CXX_FLAGS_INIT "-g -fPIC -mcpu=cortex-a53 -mfix-cortex-a53-835769 -mfix-cortex-a53-843419")

add_compile_definitions(-DPLATFORM_V6X=1)
