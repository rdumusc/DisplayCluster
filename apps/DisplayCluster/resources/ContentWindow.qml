import QtQuick 1.1
import DisplayCluster 1.0
import "qrc:/qml/core/."

Rectangle {
    id: rootObj

    color: "#80000000"
    border.color: "black"
    border.width: 10

    x: contentwindow.x
    y: contentwindow.y
    width: contentwindow.width
    height: contentwindow.height

    Binding {
        target: contentwindow; property: "x"; value: x
    }
    Binding {
        target: contentwindow; property: "y"; value: y
    }
    Binding {
        target: contentwindow; property: "width"; value: width
    }
    Binding {
        target: contentwindow; property: "height"; value: height
    }

    ContentWindowTouchArea {
        objectName: "ContentWindowTouchArea"
        id: contentWindowTouchArea
        anchors.fill: parent

        onMoveToFront: controlsFadeAnimation.restart()

        WindowControls {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 30
            onClose: contentWindowTouchArea.close()
            onToggleFullscreen: controller.toggleFullscreen()
            opacity: 0.1
            visible: contentwindow.controlsOpacity > 0 && contentwindow.label != "Dock"
        }

        Text {
            id: contentLabel
            text: contentwindow.label
            font.pixelSize: 48
            width: Math.min(paintedWidth, parent.width)
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: 10
            anchors.leftMargin: 10
        }

        Image {
            id: close
            source: "qrc:///img/master/close.svg"
            width: 0.4 * Math.min(parent.width, parent.height)
            height: width
            anchors.top: parent.top
            anchors.right: parent.right
            // Force redraw the SVG
            sourceSize.width: width
            sourceSize.height: height
            MouseArea {
                anchors.fill: parent
                preventStealing: true
                onClicked: contentWindowTouchArea.close()
            }
        }

        Image {
            id: resize
            source: "qrc:///img/master/resize.svg"
            width: 0.4 * Math.min(parent.width, parent.height)
            height: width
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            // Force redraw the SVG
            sourceSize.width: width
            sourceSize.height: height
            MouseArea {
                anchors.fill: parent
                preventStealing: true
                property variant startMousePos
                property variant startSize
                onPressed: {
                    startMousePos = Qt.point(mouse.x, mouse.y);
                    startSize = Qt.size(contentwindow.width, contentwindow.height)
                    contentwindow.state = ContentWindow.RESIZING
                }
                onPositionChanged: {
                    var newSize = Qt.size(mouse.x - startMousePos.x + startSize.width,
                                          mouse.y - startMousePos.y + startSize.height)
                    controller.resize(newSize);
                }
                onReleased: contentwindow.state = ContentWindow.NONE
            }
        }

        Image {
            id: maximize
            source: "qrc:///img/master/maximize.svg"
            width: 0.4 * Math.min(parent.width, parent.height)
            height: width
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            // Force redraw the SVG
            sourceSize.width: width
            sourceSize.height: height
            MouseArea {
                anchors.fill: parent
                preventStealing: true
                onClicked: controller.toggleFullscreen()
            }
        }
    }

    states: [
        State {
            name: "selected"
            when: contentwindow.state == ContentWindow.SELECTED
            PropertyChanges { target: rootObj; border.color: "red" }
        },
        State {
            name: "moving"
            when: contentwindow.state == ContentWindow.MOVING
            PropertyChanges { target: rootObj; border.color: "green" }
        },
        State {
            name: "resizing"
            when: contentwindow.state == ContentWindow.RESIZING
            PropertyChanges { target: rootObj; border.color: "blue" }
        },
        State {
            name: "hidden"
            when: contentwindow.state == ContentWindow.HIDDEN
            PropertyChanges { target: rootObj; visible: false }
        }
    ]

    NumberAnimation {
        id: controlsFadeAnimation
        target: contentwindow
        property: "controlsOpacity"
        from: 1
        to: 0
        duration: 2000
        easing.type: Easing.InExpo
    }
}
