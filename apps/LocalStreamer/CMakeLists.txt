
# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

set(LOCALSTREAMER_MOC_HEADERS
  Application.h
)

set(LOCALSTREAMER_SOURCES
  main.cpp
  Application.cpp
)

set(LOCALSTREAMER_LINK_LIBRARIES dccore Deflect)

common_application(localstreamer)
