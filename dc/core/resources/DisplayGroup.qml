import QtQuick 1.1
import DisplayCluster 1.0

Item {
    property alias showFocus: focuscontext.visible
    id: displaygroupitem
    x: displaygroup.x
    y: displaygroup.y
    width: displaygroup.width
    height: displaygroup.height

    Rectangle {
        id: focuscontext
        anchors.fill: parent
        color: "transparent"
        z: 100

        states: [
            State {
                name: "focused"
                when: displaygroup.hasFocusedWindows
                PropertyChanges { target: focuscontext; color: "#aa000000" }
            }
        ]
        transitions: [
            Transition {
                to: "*"
                ColorAnimation { target: focuscontext; duration: 400 }
            }
        ]
    }
}
