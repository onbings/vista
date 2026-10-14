# =========================
# == START OF DISCLAIMER ==
# =========================

# Do not modify any default CMake behaviour here.
# Example : By default, CMake build static libraries.
#  Even if you like it or not this is the default !
#  Don't change that !
#
# The intent of this file is to provide options for
# ADDITIONAL features

# =======================
# == END OF DISCLAIMER ==
# =======================

# clang-tidy does not work
# well with sysrooted environment
if(DEFINED CMAKE_SYSROOT)
  set(DEFAULT_ENABLE_CLANGTIDY OFF)
else()
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEFAULT_ENABLE_CLANGTIDY ON)
  else()
    set(DEFAULT_ENABLE_CLANGTIDY OFF)
  endif()
endif()

option(ENABLE_CLANGTIDY  "Enable the clang-tidy linter tool"    ${DEFAULT_ENABLE_CLANGTIDY})
option(ENABLE_IWYU       "Enable the include-what-you-use tool" OFF)
