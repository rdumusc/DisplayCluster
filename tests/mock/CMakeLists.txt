
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

qt4_wrap_cpp(MOC_OUTFILES ${DCMOCK_MOC_HEADERS})

add_library(dcmock SHARED ${MOC_OUTFILES} ${DCMOCK_HEADERS} ${DCMOCK_SOURCES})
set_target_properties(dcmock PROPERTIES
  COMPILE_DEFINITIONS "DISPLAYCLUSTER_SHARED")
target_link_libraries(dcmock ${DC_LIBRARIES})
