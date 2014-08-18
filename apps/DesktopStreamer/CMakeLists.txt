
set(DESKTOP_STREAMER_SRCS ${DESKTOP_STREAMER_SRCS}
    src/DesktopSelectionRectangle.cpp
    src/DesktopSelectionWindow.cpp
    src/DesktopSelectionView.cpp
    src/main.cpp
    src/MainWindow.cpp
)

set(DESKTOP_STREAMER_MOC_HEADERS
    src/MainWindow.h
    src/DesktopSelectionWindow.h
    src/DesktopSelectionRectangle.h
)

qt4_wrap_cpp(DESKTOP_STREAMER_MOC_OUTFILES ${DESKTOP_STREAMER_MOC_HEADERS})
qt4_add_resources(DESKTOP_STREAMER_RRC resources.qrc)

set(DESKTOP_STREAMER_LIBS ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

if(APPLE)
  set(STREAMER_APP_NAME DesktopStreamer)
  set(STREAMER_ICON_FILE desktopstreamer.icns)

  # Required to control AppNap on OSX >= 10.9
  list(APPEND DESKTOP_STREAMER_LIBS "-framework Foundation")
  list(APPEND DESKTOP_STREAMER_SRCS src/AppNapSuspender.mm)

  # set where in the bundle to put the icns file
  set_source_files_properties(${STREAMER_ICON_FILE}
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

  # include the icns file in the target
  add_executable(${STREAMER_APP_NAME} MACOSX_BUNDLE ${STREAMER_ICON_FILE}
    ${DESKTOP_STREAMER_SRCS} ${DESKTOP_STREAMER_MOC_OUTFILES} ${DESKTOP_STREAMER_RRC})
  set_target_properties(${STREAMER_APP_NAME} PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
elseif(WIN32)
  set(STREAMER_APP_NAME desktopstreamer)
  list(APPEND DESKTOP_STREAMER_LIBS ${QT_QTMAIN_LIBRARY})

  add_executable(${STREAMER_APP_NAME} WIN32
    ${DESKTOP_STREAMER_SRCS} ${DESKTOP_STREAMER_MOC_OUTFILES} ${DESKTOP_STREAMER_RRC})
else()
  set(STREAMER_APP_NAME desktopstreamer)

  add_executable(${STREAMER_APP_NAME}
    ${DESKTOP_STREAMER_SRCS} ${DESKTOP_STREAMER_MOC_OUTFILES} ${DESKTOP_STREAMER_RRC})
endif()

# Note: dcstream is a target, which needs to be handled differently above
target_link_libraries(${STREAMER_APP_NAME} ${DESKTOP_STREAMER_LIBS} dcstream)

# install executable
install(TARGETS ${STREAMER_APP_NAME}
  RUNTIME DESTINATION bin COMPONENT desktopstreamer
  BUNDLE DESTINATION . COMPONENT desktopstreamer)

include(CPackConfig)
