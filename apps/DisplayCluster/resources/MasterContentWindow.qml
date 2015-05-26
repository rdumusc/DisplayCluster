import QtQuick 1.1
import DisplayCluster 1.0
import DisplayClusterApp 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

BaseContentWindow {
    color: "#80000000"
    border.width: 10

    function closeWindow() {
        displaygroup.removeContentWindow(contentwindow.id)
    }

    ContentWindowTouchArea {
        objectName: "ContentWindowTouchArea"
        anchors.fill: parent
        focus: true // to receive key events
        onActivated: {
            displaygroup.moveContentWindowToFront(contentwindow.id)
            controlsFadeAnimation.restart()
        }
    }

    WindowBorders {
        borderDelegate: touchBorderDelegate

        Component {
            id: touchBorderDelegate
            BorderRectangle {
                TouchArea {
                    anchors.fill: parent
                    onPan: {
                        controlsFadeAnimation.stop()
                        contentwindow.border = parent.border
                        contentwindow.state = ContentWindow.RESIZING
                        controller.resizeRelative(delta)
                    }
                    onPanFinished: {
                        controlsFadeAnimation.restart()
                        contentwindow.border = ContentWindow.NOBORDER
                        contentwindow.state = ContentWindow.NONE
                    }
                }
            }
        }
    }

    WindowControls {
        listview.delegate: buttonDelegate
        listview.header: fixedButtonsDelegate

        Component {
            id: fixedButtonsDelegate
            Column {
                CloseControlButton {
                    TouchArea {
                        anchors.fill: parent
                        onTap: closeWindow()
                    }
                }
                OneToOneControlButton {
                    TouchArea {
                        anchors.fill: parent
                        onTap: {
                            controlsFadeAnimation.restart()
                            controller.adjustSizeOneToOne()
                        }
                    }
                }
                FullscreenControlButton {
                    TouchArea {
                        anchors.fill: parent
                        onTap: {
                            controlsFadeAnimation.restart()
                            controller.toggleFullscreen()
                        }
                    }
                }
            }
        }

        Component {
            id: buttonDelegate
            WindowControlsDelegate {
                TouchArea {
                    anchors.fill: parent
                    onTap: {
                        controlsFadeAnimation.restart()
                        action.trigger()
                    }
                }
            }
        }
    }

    Text {
        visible: !parent.titleBar.visible
        text: contentwindow.label
        font.pixelSize: 48
        width: Math.min(paintedWidth, parent.width)
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: 10
        anchors.leftMargin: 10
    }

    ContentWindowButton {
        source: "qrc:///img/master/close.svg"
        anchors.top: parent.top
        anchors.right: parent.right
        mousearea.onClicked: closeWindow()
    }

    ContentWindowButton {
        source: "qrc:///img/master/maximize.svg"
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        mousearea.onClicked: controller.toggleFullscreen()
    }

    ContentWindowButton {
        source: "qrc:///img/master/resize.svg"
        anchors.bottom: parent.bottom
        anchors.right: parent.right

        property variant startMousePos
        property variant startSize
        mousearea.onPressed: {
            startMousePos = Qt.point(mouse.x, mouse.y)
            startSize = Qt.size(contentwindow.width, contentwindow.height)
            contentwindow.state = ContentWindow.RESIZING
        }
        mousearea.onPositionChanged: {
            var newSize = Qt.size(mouse.x - startMousePos.x + startSize.width,
                                  mouse.y - startMousePos.y + startSize.height)
            controller.resize(newSize)
        }
        mousearea.onReleased: contentwindow.state = ContentWindow.NONE
    }

    NumberAnimation {
        id: controlsFadeAnimation
        target: contentwindow
        property: "controlsOpacity"
        from: 1
        to: 0
        duration: Style.controlsFadeOutTime
        easing.type: Easing.InExpo
    }
}
