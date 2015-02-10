import QtQuick 1.1
import DisplayCluster 1.0

Rectangle {
    id: rootObj
    color: "transparent"

    x: contentwindow.x
    y: contentwindow.y
    width: contentwindow.width
    height: contentwindow.height

    WindowControls {
        visible: contentwindow.label != "Dock"
        opacity: contentwindow.controlsOpacity
    }

    states: [
        State {
            name: "hidden"
            when: contentwindow.state == ContentWindow.HIDDEN
            PropertyChanges { target: rootObj; visible: false }
        }
    ]
}
