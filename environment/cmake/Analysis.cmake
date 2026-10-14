# Create compile_command.json that
# can be used by clang tooling
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# clang-tidy
if(ENABLE_CLANGTIDY)
  include(${CMAKE_CURRENT_LIST_DIR}/Analysis/clang_tidy.cmake)
endif()

# include-what-you-use
if(ENABLE_IWYU)
  include(${CMAKE_CURRENT_LIST_DIR}/Analysis/iwyu.cmake)
endif()
