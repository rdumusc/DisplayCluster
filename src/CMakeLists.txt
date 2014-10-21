
# Copyright (c) 2013-2014, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)
include_directories(${PROJECT_BINARY_DIR}) ### for config.h ###

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FI ${PROJECT_BINARY_DIR}/config.h")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include config.h")
endif()

set(DISPLAYCLUSTER_SHARED_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/Event.h
  ${CMAKE_CURRENT_SOURCE_DIR}/MessageHeader.h
  ${CMAKE_CURRENT_SOURCE_DIR}/NetworkProtocol.h
  ${CMAKE_CURRENT_SOURCE_DIR}/PixelStreamSegment.h
  ${CMAKE_CURRENT_SOURCE_DIR}/PixelStreamSegmentParameters.h
)

set(DISPLAYCLUSTER_SHARED_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/Event.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/log.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/MessageHeader.cpp
)

if(BUILD_CORE_LIBRARY)
  add_subdirectory(core)
  add_subdirectory(dcWebservice)
endif()

add_subdirectory(dcstream)
