
# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>
#
# Generates a mock library used by the cpp unit tests.

set(DCMOCK_HEADERS
  glxDisplay.h
  glVersion.h
  GlobalQtApp.h
  MinimalGlobalQtApp.h
)

set(DCMOCK_MOC_HEADERS MockTextInputDispatcher.h)
set(DCMOCK_SOURCES MockTextInputDispatcher.cpp)
set(DCMOCK_LINK_LIBRARIES ${DC_LIBRARIES} ${X11_LIBRARIES})

set(DCMOCK_INCLUDE_NAME dc/mock)
set(DCMOCK_OMIT_LIBRARY_HEADER ON)
common_library(dcmock)
