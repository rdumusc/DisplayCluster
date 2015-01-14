import QtQuick 1.1
import DisplayCluster 1.0

Rectangle {
    id: rootObj

    color: "grey"
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

//        MouseArea {
//            anchors.fill: parent
//            onDoubleClicked: contentwindow.toggleSelectedState()
//            drag.target: rootObj
//            preventStealing: true
//        }

//        Rectangle {
//            id: closeButton
//            width: 0.23 * parent.width
//            height: 0.45 * width
//            color: "blue"
//            anchors.horizontalCenter: parent.horizontalCenter
//            anchors.bottom: parent.bottom
//            anchors.bottomMargin: 20
//            MouseArea {
//                anchors.fill: parent
//                preventStealing: true
//                onClicked: contentWindowItem.close()
//            }
//        }

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
                onClicked: contentWindowItem.toggleFullscreen()
            }
        }
    }

    states: [
        State {
            name: "selected"
            when: contentwindow.selected
            PropertyChanges { target: rootObj; border.color: "red" }
        },
        State {
            name: "hidden"
            when: contentwindow.hidden
            PropertyChanges { target: rootObj; visible: false }
        },
        State {
            name: "resizing"
            when: contentwindow.resizing
            PropertyChanges { target: rootObj; border.color: "blue" }
        },
        State {
            name: "moving"
            when: contentwindow.moving
            PropertyChanges { target: rootObj; border.color: "green" }
        }
    ]

}
