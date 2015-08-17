import QtQuick 1.1
import DisplayCluster 1.0
import "qrc:/qml/core/style.js" as Style

Item {
    property alias showFocusContext: focuscontext.visible
    id: displaygroupitem
    x: displaygroup.x
    y: displaygroup.y
    width: displaygroup.width
    height: displaygroup.height

    Rectangle {
        id: focuscontext
        anchors.fill: parent
        color: "black"
        opacity: 0
        visible: opacity > 0
        z: 100
        states: [
            State {
                name: "focused"
                when: displaygroup.hasFocusedWindows
                PropertyChanges {
                    target: focuscontext
                    opacity: Style.focusContextOpacity
                }
            }
        ]
        Behavior on opacity {
            NumberAnimation {
                target: focuscontext
                property: "opacity"
                duration: Style.focusTransitionTime
                easing.type: Easing.InOutQuad
            }
        }
    }
}
