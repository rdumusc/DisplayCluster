import QtQuick 1.1
import DisplayCluster 1.0

Rectangle {
    id: rootObj

    color: "transparent"
//    border.color: "black"
//    border.width: 10

    x: contentwindow.x
    y: contentwindow.y
    width: contentwindow.width
    height: contentwindow.height

    WindowControls {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 30
        visible: contentwindow.label != "Dock"
        opacity: contentwindow.controlsOpacity
    }

    states: [
//        State {
//            name: "selected"
//            when: contentwindow.state == ContentWindow.SELECTED
//            PropertyChanges { target: rootObj; border.color: "red" }
//        },
//        State {
//            name: "moving"
//            when: contentwindow.state == ContentWindow.MOVING
//            PropertyChanges { target: rootObj; border.color: "green" }
//        },
//        State {
//            name: "resizing"
//            when: contentwindow.state == ContentWindow.RESIZING
//            PropertyChanges { target: rootObj; border.color: "blue" }
//        },
        State {
            name: "hidden"
            when: contentwindow.state == ContentWindow.HIDDEN
            PropertyChanges { target: rootObj; visible: false }
        }
    ]
}
