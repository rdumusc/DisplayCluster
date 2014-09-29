
# Copyright (c) 2013-2014, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

if(BUILD_CORE_LIBRARY)
  add_subdirectory(DisplayCluster)
  add_subdirectory(LocalStreamer)
endif()

add_subdirectory(DesktopStreamer)

if(GLUT_FOUND)
  add_subdirectory(SimpleStreamer)
endif()
