import QtQuick 1.1
import DisplayCluster 1.0
import DisplayClusterApp 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

BaseContentWindow {
    objectName: "MasterContentWindow"
    color: "#80000000"
    border.width: 10

    function closeWindow() {
        displaygroup.removeContentWindow(contentwindow.id)
    }

    function toggleControlsVisibility() {
        contentwindow.controlsVisible = !contentwindow.controlsVisible
    }

    function toggleFocusMode() {
        if(contentwindow.label === "Dock")
            return
        if(contentwindow.focused)
            displaygroup.unfocus(contentwindow.id)
        else
            displaygroup.focus(contentwindow.id)
    }

    TouchMouseArea {
        focus: true // to receive key events
        anchors.fill: parent
        onTouchBegin: {
            displaygroup.moveContentWindowToFront(contentwindow.id)
        }
        onTap: {
            if(contentwindow.state === ContentWindow.SELECTED)
                contentwindow.delegate.tap(position)
            else
                toggleControlsVisibility()
        }
        onDoubleTap: {
            toggleFocusMode()
        }
        onTapAndHold: {
            if(contentwindow.state === ContentWindow.SELECTED)
                contentwindow.delegate.tapAndHold(position)
        }
        onPan: {
            if(contentwindow.state === ContentWindow.SELECTED)
                contentwindow.delegate.pan(position, delta)
            else {
                contentwindow.state = ContentWindow.MOVING
                contentwindow.controller.moveTo(Qt.point(contentwindow.x + delta.x,
                                                         contentwindow.y + delta.y))
            }
        }
        onPanFinished: {
            if(contentwindow.state !== ContentWindow.SELECTED)
                contentwindow.state = ContentWindow.NONE
        }
        onPinch: {
            if(contentwindow.state === ContentWindow.SELECTED)
                contentwindow.delegate.pinch(position, scaleFactor)
            else {
                contentwindow.state = ContentWindow.RESIZING
                contentwindow.controller.scale(position, scaleFactor)
            }
        }
        onPinchFinished: {
            if(contentwindow.state !== ContentWindow.SELECTED)
                contentwindow.state = ContentWindow.NONE
        }
        onSwipeLeft: {
            if(contentwindow.state === ContentWindow.SELECTED)
                contentwindow.delegate.swipeLeft()
        }
        onSwipeRight: {
            if(contentwindow.state === ContentWindow.SELECTED)
                contentwindow.delegate.swipeRight()
        }
        onSwipeUp: {
            if(contentwindow.state === ContentWindow.SELECTED)
                contentwindow.delegate.swipeUp()
        }
        onSwipeDown: {
            if(contentwindow.state === ContentWindow.SELECTED)
                contentwindow.delegate.swipeDown()
        }
        onKeyPress: {
            if(contentwindow.state === ContentWindow.SELECTED)
                contentwindow.delegate.keyPress(key, modifiers, text)
        }
        onKeyRelease: {
            if(contentwindow.state === ContentWindow.SELECTED)
                contentwindow.delegate.keyRelease(key, modifiers, text)
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
                        contentwindow.border = parent.border
                        contentwindow.state = ContentWindow.RESIZING
                        contentwindow.controller.resizeRelative(delta)
                    }
                    onPanFinished: {
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
                            contentwindow.controller.adjustSizeOneToOne()
                        }
                    }
                }
                FocusControlButton {
                    TouchArea {
                        anchors.fill: parent
                        onTap: {
                            toggleFocusMode()
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
        mousearea.onClicked: toggleFocusMode()
    }

    ContentWindowButton {
        source: "qrc:///img/master/resize.svg"
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        visible: contentwindow.state !== ContentWindow.SELECTED

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
            contentwindow.controller.resize(newSize)
        }
        mousearea.onReleased: contentwindow.state = ContentWindow.NONE
    }
}
