set(TOOLSET_VERSION           "${MSVC_TOOLSET_VERSION}")
set(TOOLSET_COMPILER_ARCH     "${CMAKE_SYSTEM_PROCESSOR}")
set(TOOLSET_COMPILER_VERSION  "msvc_${MSVC_VERSION}")

# Add the compile flags for the toolset
include(${CMAKE_CURRENT_LIST_DIR}/compile_flags.cmake)

# The other modules are called by their respective "main" module


