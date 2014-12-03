
# Copyright (c) 2013-2014, EPFL/Blue Brain Project
#                     Daniel Nachbaur <daniel.nachbaur@epfl.ch>
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>
#
# Generates a mock library used by the cpp unit tests.

set(MOCK_HEADERS
  glxDisplay.h
  glVersion.h
  GlobalQtApp.h
  MinimalGlobalQtApp.h
)

set(MOCK_MOC_HEADERS
  MockNetworkListener.h
)

set(MOCK_SOURCES
  MockNetworkListener.cpp
)

if(BUILD_CORE_LIBRARY)
  list(APPEND MOCK_MOC_HEADERS MockTextInputDispatcher.h)
  list(APPEND MOCK_SOURCES MockTextInputDispatcher.cpp)
endif()

qt4_wrap_cpp(MOC_OUTFILES ${MOCK_MOC_HEADERS})

add_library(mock SHARED ${MOC_OUTFILES} ${MOCK_HEADERS} ${MOCK_SOURCES})
set_target_properties(mock PROPERTIES
  COMPILE_DEFINITIONS "DISPLAYCLUSTER_SHARED")
target_link_libraries(mock ${DC_LIBRARIES})
