set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "$ENV{P6X_TOOLCHAIN_FILE}")

set(VCPKG_MESON_CROSS_FILE "${CMAKE_CURRENT_LIST_DIR}/../../dockers/toolchains/p6x_meson_cross_compile.txt")

# dbus requires DBUS_SESSION_SOCKET_DIR when cross-compiling
if(PORT STREQUAL "dbus")
    list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS "-DDBUS_SESSION_SOCKET_DIR=/tmp")
endif()

# ffmpeg's ./configure locates libvpl (qsv feature) via pkg-config. PKG_CONFIG_SYSROOT_DIR
# (set by the chainloaded SDK toolchain) gets prepended to every .pc path it finds, which
# corrupts vpl.pc's absolute vcpkg path (libvpl is vcpkg-provided, not part of the sysroot).
# Clear it for this port only; ffmpeg's remaining enabled deps all come from vcpkg.
if(PORT STREQUAL "ffmpeg")
    set(ENV{PKG_CONFIG_SYSROOT_DIR} "")
endif()

