
# Copyright (c) 2013-2014, EPFL/Blue Brain Project
#                     Raphael Dumusc <raphael.dumusc@epfl.ch>

set(DCWEBSERVICE_PUBLIC_HEADERS
  DefaultHandler.h
  FastCGIWrapper.h
  Handler.h
  Mapper.h
  Request.h
  RequestBuilder.h
  Response.h
  Server.h
  types.h
)

set(DCWEBSERVICE_SOURCES
  DefaultHandler.cpp
  FastCGIWrapper.cpp
  Mapper.cpp
  RequestBuilder.cpp
  Response.cpp
  Server.cpp
)

set(DCWEBSERVICE_LINK_LIBRARIES ${Boost_LIBRARIES} ${FCGI_LIBRARIES})

add_library(dcwebservice SHARED ${DCWEBSERVICE_SOURCES} ${DCWEBSERVICE_HEADERS})
target_link_libraries(dcwebservice ${DCWEBSERVICE_LINK_LIBRARIES})
set_target_properties(dcwebservice PROPERTIES
  VERSION ${VERSION} SOVERSION ${VERSION_ABI})

install(TARGETS dcwebservice
  ARCHIVE DESTINATION lib COMPONENT dev
  RUNTIME DESTINATION bin COMPONENT core
  LIBRARY DESTINATION lib COMPONENT core)

install(FILES ${DCWEBSERVICE_PUBLIC_HEADERS} DESTINATION include/dc/webservice
  COMPONENT dev)
