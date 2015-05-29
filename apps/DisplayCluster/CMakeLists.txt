
# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${PROJECT_BINARY_DIR}) ### for config.h ###

### for override on pre-C++11 compilers ###
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FI ${PROJECT_BINARY_DIR}/config.h")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include config.h")
endif()

set(DISPLAYCLUSTER_MOC_HEADERS
  BackgroundWidget.h
  ContentWindowTouchArea.h
  DisplayGroupGraphicsView.h
  DisplayGroupListWidget.h
  MasterApplication.h
  MasterWindow.h
  TouchArea.h
  WallApplication.h
  WebbrowserWidget.h
)

set(DISPLAYCLUSTER_SOURCES
  main.cpp
  BackgroundWidget.cpp
  CommandLineParameters.cpp
  ContentWindowTouchArea.cpp
  ContentWindowListWidgetItem.cpp
  DisplayGroupGraphicsScene.cpp
  DisplayGroupGraphicsView.cpp
  DisplayGroupListWidget.cpp
  MasterApplication.cpp
  MasterWindow.cpp
  QmlTypeRegistration.cpp
  TouchArea.cpp
  WallApplication.cpp
  WebbrowserWidget.cpp
)

set(DISPLAYCLUSTER_LINK_LIBRARIES dccore dcwebservice)
set(DISPLAYCLUSTER_RESOURCES resources/displaycluster.qrc)

common_application(displaycluster)
install(PROGRAMS startdisplaycluster DESTINATION bin COMPONENT apps)

# Copy the startup script and default config to the bin folder to be able to
# launch after building without requiring an install folder and 'make install'
file(COPY startdisplaycluster DESTINATION ${PROJECT_BINARY_DIR}/bin)
file(COPY ${PROJECT_SOURCE_DIR}/examples/configuration.xml
  DESTINATION ${PROJECT_BINARY_DIR}/share/DisplayCluster/examples
)
