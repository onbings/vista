set(CMAKE_SYSTEM_NAME      Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_SYSROOT          /opt/sdk/p6x/x86_64-buildroot-linux-gnu/sysroot)
set(CMAKE_C_COMPILER       /opt/sdk/p6x/bin/x86_64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER     /opt/sdk/p6x/bin/x86_64-linux-gnu-g++)

set(ENV{PKG_CONFIG_SYSROOT_DIR} "${CMAKE_SYSROOT}")
set(ENV{PKG_CONFIG_PATH}        "${CMAKE_SYSROOT}/usr/lib/pkgconfig:${CMAKE_SYSROOT}/usr/share/pkgconfig")

set(CMAKE_C_FLAGS_INIT   "-g -fPIC -mtune=alderlake")
set(CMAKE_CXX_FLAGS_INIT "-g -fPIC -mtune=alderlake")

add_compile_definitions(-DPLATFORM_P6X=1)
