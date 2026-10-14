
# =============
# == SOURCES ==
# =============

set(SRC_FILES
  src/main.cpp
  src/vista_daemon.cpp
  src/vista_daemon.h
)

# Logical groups
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "src" FILES ${SRC_FILES})

# ===========
# == BUILD ==
# ===========

add_executable(onbings-vista-service
  ${SRC_FILES}
)

# Include
target_include_directories(onbings-vista-service
  PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)

# Link
target_link_libraries(onbings-vista-service
  PRIVATE
  EVS::evs-hwfw-ilf
  EVS::evs-hwfw-ilf-backend-spdlog
  ONBINGS::onbings-vista
  ONBINGS::bofstd
)

# Defines
target_compile_definitions(onbings-vista-service
  PRIVATE
)

# Feature
target_compile_features(onbings-vista-service
  PRIVATE
)

# =============
# == INSTALL ==
# =============

# Install the target runtime
install(
  TARGETS onbings-vista-service
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # For binaries and dll
  COMPONENT onbings-vista-runtime
)

# ==============
# == APPIMAGE ==
# ==============

evs_generate_manifest(
  TARGET onbings-vista-service
)

if(ONBINGS_VISTA_BUILD_APPIMAGES)
  message("====== APP IMAGE add file/dir to assets dir ======>" ${CMAKE_SOURCE_DIR}/code/sources/onbings-vista-lib/assets)
  evs_make_appimage(
    TARGET onbings-vista-service
    #COPY_FILE ${CMAKE_BINARY_DIR}/assets/rest-vista.html bin/assets
    COPY_DIR  ${CMAKE_SOURCE_DIR}/code/sources/onbings-vista-lib/assets bin
    FORCE_STRIP    
  )
endif()