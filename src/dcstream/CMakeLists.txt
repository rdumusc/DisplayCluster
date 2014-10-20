
# Copyright (c) 2013-2014, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

set(DCSTREAM_PUBLIC_HEADERS
  ${COMMON_INCLUDES}
  ${DISPLAYCLUSTER_SHARED_HEADERS}
  ImageWrapper.h
  Stream.h
  types.h
)

set(DCSTREAM_MOC_HEADERS
  Socket.h
)

set(DCSTREAM_SOURCES
  ${DISPLAYCLUSTER_SHARED_SOURCES}
  Socket.cpp
  Stream.cpp
  StreamPrivate.cpp
  StreamSendWorker.cpp
  ImageWrapper.cpp
  ImageSegmenter.cpp
  ImageJpegCompressor.cpp
)

set(DCSTREAM_LINK_LIBRARIES
  ${QT_QTCORE_LIBRARY}
  ${QT_QTNETWORK_LIBRARY}
  ${LibJpegTurbo_LIBRARIES}
  ${Boost_THREAD_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
)

qt4_wrap_cpp(MOC_OUTFILES ${DCSTREAM_MOC_HEADERS})

add_library(dcstream SHARED ${DCSTREAM_PUBLIC_HEADERS} ${DCSTREAM_SOURCES}
            ${MOC_OUTFILES})
target_link_libraries(dcstream ${DCSTREAM_LINK_LIBRARIES})
set_target_properties(dcstream PROPERTIES
  VERSION ${VERSION} SOVERSION ${VERSION_ABI}
  COMPILE_DEFINITIONS "DISPLAYCLUSTER_SHARED")

install(TARGETS dcstream
  ARCHIVE DESTINATION lib COMPONENT dev       # .a, .lib
  RUNTIME DESTINATION bin COMPONENT dcstream  # .exe, .dll
  LIBRARY DESTINATION lib COMPONENT dcstream  # .so, mod.dll
)

install(FILES ${DCSTREAM_PUBLIC_HEADERS} DESTINATION include/dc COMPONENT dev)
