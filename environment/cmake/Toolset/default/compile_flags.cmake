#=========================
#== START OF DISCLAIMER ==
#=========================

# This file should only contains the compilation flags (or related) used by the whole solution

#========================
#== STOP OF DISCLAIMER ==
#========================

add_compile_options(
  $<$<COMPILE_LANGUAGE:C,CXX>:-pedantic>            # Issue all the warnings demanded by strict ISO C and ISO C++
  $<$<COMPILE_LANGUAGE:C,CXX>:-Wall>                # This enables all the warnings about constructions that some users consider questionable
  $<$<COMPILE_LANGUAGE:C,CXX>:-Wcast-align=strict>  # Warn about casting to unaligned memory
  $<$<COMPILE_LANGUAGE:C,CXX>:-Wchar-subscripts>    # Warn if an array subscript has type char
  $<$<COMPILE_LANGUAGE:C,CXX>:-Wformat>             # Check calls to printf and scanf, etc.
  $<$<COMPILE_LANGUAGE:C,CXX>:-Wextra>              # This enables some extra warning flags that are not enabled by -Wall.
  $<$<COMPILE_LANGUAGE:C,CXX>:-Wno-psabi>           # Disable warning about potential ABI breaks in GCC
  $<$<COMPILE_LANGUAGE:C,CXX>:-Wmultichar>          # Warn if a multichar character constant is used (e.g. 'FOOF')
  $<$<COMPILE_LANGUAGE:C,CXX>:-Wpointer-arith>      # Warn about anything that depends on the “size of” a function type or of void. 
  $<$<COMPILE_LANGUAGE:C,CXX>:-Wreturn-type>        # Warn whenever a function is defined with a return type that defaults to int 
  $<$<COMPILE_LANGUAGE:C,CXX>:-Wswitch-enum>        # Warn whenever a switch statement has an index of enumerated type and lacks a case
  
  $<$<COMPILE_LANGUAGE:C,CXX>:-ggdb3>               # Always compile with debbuging info
)
