
# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

include_directories(${CMAKE_SOURCE_DIR}/src/core)

set(PYRAMIDMAKER_SOURCES
  main.cpp
)

set(PYRAMIDMAKER_LINK_LIBRARIES dccore)

add_executable(pyramidmaker ${PYRAMIDMAKER_SOURCES})
target_link_libraries(pyramidmaker ${PYRAMIDMAKER_LINK_LIBRARIES})

install(TARGETS pyramidmaker RUNTIME DESTINATION bin)
