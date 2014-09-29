
# Copyright (c) 2013-2014, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${CMAKE_SOURCE_DIR}/src/core)
include_directories(${CMAKE_SOURCE_DIR}/src/dcstream)
include_directories(${PROJECT_BINARY_DIR}) ### for config.h ###

set(LOCALSTREAMER_MOC_HEADERS
  src/Application.h
)

set(LOCALSTREAMER_SOURCES
  ../../src/log.cpp
  src/main.cpp
  src/Application.cpp
)

set(LOCALSTREAMER_LINK_LIBRARIES dccore dcstream)

qt4_wrap_cpp(MOC_OUTFILES ${LOCALSTREAMER_MOC_HEADERS})

add_executable(localstreamer ${MOC_OUTFILES} ${LOCALSTREAMER_SOURCES})
target_link_libraries(localstreamer ${LOCALSTREAMER_LINK_LIBRARIES})

install(TARGETS localstreamer RUNTIME DESTINATION bin)
