# Copyright (c) 2016, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

include(zerobufGenerateCxx)
zerobuf_generate_cxx(REST ${OUTPUT_INCLUDE_DIR}/${DCCORE_INCLUDE_NAME}
  command.fbs
)

list(APPEND DCCORE_MOC_HEADERS
  RESTServer.cpp
)

list(APPEND DCCORE_SOURCES
  RESTServer.cpp
)
