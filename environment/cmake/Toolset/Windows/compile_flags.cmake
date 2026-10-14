#=========================
#== START OF DISCLAIMER ==
#=========================

# This file should only contains the compilation flags (or related) used by the whole solution

#========================
#== STOP OF DISCLAIMER ==
#========================

if (MSVC)

  # Activate building in //
  add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/MP>)

  # Add following warning :
  #  C4061 "Warn about missing case in switch on enums with default" -> https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4061?view=msvc-170 
  add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/w44061>)
  
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /we4061" CACHE INTERNAL "")
  
  
endif()