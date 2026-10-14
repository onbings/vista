set(CMAKE_SYSTEM_NAME      Linux)
set(CMAKE_SYSTEM_PROCESSOR armhf)

set(CMAKE_SYSROOT          /opt/sdk/r6x/arm-evs-linux-gnueabihf/sysroot)
set(CMAKE_C_COMPILER       /opt/sdk/r6x/bin/arm-evs-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER     /opt/sdk/r6x/bin/arm-evs-linux-gnueabihf-g++)

set(CMAKE_C_FLAGS_INIT   "-g -fPIC")
set(CMAKE_CXX_FLAGS_INIT "-g -fPIC")

add_compile_definitions(-DPLATFORM_R6X=1)
