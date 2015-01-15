import QtQuick 1.1
import DisplayCluster 1.0

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

    ContentWindowItem {
        objectName: "contentWindowItem"
        id: contentWindowItem
        anchors.fill: parent

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
            source: "qrc:///img/masterui-close.svg"
            width: 0.23 * parent.width
            height: width
            anchors.top: parent.top
            anchors.right: parent.right
            // Force redraw the SVG
            sourceSize.width: width
            sourceSize.height: height
            MouseArea {
                anchors.fill: parent
                preventStealing: true
                onClicked: contentWindowItem.close()
            }
        }

        Image {
            id: resize
            source: "qrc:///img/masterui-resize.svg"
            width: 0.23 * parent.width
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
            source: "qrc:///img/masterui-maximize.svg"
            width: 0.23 * parent.width
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

}
