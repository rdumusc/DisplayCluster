
# Copyright (c) 2013-2015, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

set(LOCALSTREAMER_MOC_HEADERS
  Application.h
)

set(LOCALSTREAMER_SOURCES
  main.cpp
  Application.cpp
)

set(LOCALSTREAMER_LINK_LIBRARIES dccore ${DEFLECT_LIBRARIES})

qt5_wrap_cpp(MOC_OUTFILES ${LOCALSTREAMER_MOC_HEADERS})

add_executable(localstreamer ${MOC_OUTFILES} ${LOCALSTREAMER_SOURCES})
target_link_libraries(localstreamer ${LOCALSTREAMER_LINK_LIBRARIES})

install(TARGETS localstreamer RUNTIME DESTINATION bin)
