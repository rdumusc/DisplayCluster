
include_directories(${CMAKE_SOURCE_DIR}/src/core)
include_directories(${CMAKE_BINARY_DIR}) ### for config.h ###

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include config.h")

set(MOC_HEADERS
  BackgroundWidget.h
  MasterApplication.h
  MasterWindow.h
  WallApplication.h
)

set(DISPLAYCLUSTER_FILES
  main.cpp
  BackgroundWidget.cpp
  CommandLineParameters.cpp
  MasterApplication.cpp
  MasterWindow.cpp
  WallApplication.cpp
)

qt4_wrap_cpp(MOC_OUTFILES ${MOC_HEADERS})

# DisplayCluster Main Application
add_executable(displaycluster ${MOC_OUTFILES} ${DISPLAYCLUSTER_FILES})

target_link_libraries(displaycluster dccore dcwebservice)

# install executable
install(TARGETS displaycluster RUNTIME DESTINATION bin COMPONENT core)

# install launcher
install(PROGRAMS startdisplaycluster DESTINATION bin COMPONENT core)
