import QtQuick 1.1
import DisplayCluster 1.0

Rectangle {
    id: borderRect
    property int borderSize: 100
    property int border
    state: "inactive"

    anchors.top: parent.top
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.right: parent.right

    anchors.topMargin: border === ContentWindow.LEFT || border
                       === ContentWindow.RIGHT ? borderSize : border === ContentWindow.BOTTOM
                                                 || border === ContentWindow.BOTTOM_LEFT
                                                 || border === ContentWindow.BOTTOM_RIGHT ? parent.height - borderSize : 0
    anchors.bottomMargin: border === ContentWindow.LEFT || border
                          === ContentWindow.RIGHT ? borderSize : border === ContentWindow.TOP
                                                    || border === ContentWindow.TOP_LEFT
                                                    || border === ContentWindow.TOP_RIGHT ? parent.height - borderSize : 0
    anchors.leftMargin: border === ContentWindow.TOP || border
                        === ContentWindow.BOTTOM ? borderSize : border === ContentWindow.RIGHT
                                                   || border === ContentWindow.TOP_RIGHT
                                                   || border === ContentWindow.BOTTOM_RIGHT ? parent.width - borderSize : 0
    anchors.rightMargin: border === ContentWindow.TOP || border
                         === ContentWindow.BOTTOM ? borderSize : border === ContentWindow.LEFT
                                                    || border === ContentWindow.TOP_LEFT
                                                    || border === ContentWindow.BOTTOM_LEFT ? parent.width - borderSize : 0

    states: [
        State {
            name: "active"
            when: contentwindow.border === border
            PropertyChanges {
                target: borderRect
                color: "red"
            }
        },
        State {
            name: "inactive"
            when: contentwindow.border === ContentWindow.NOBORDER
            PropertyChanges {
                target: borderRect
                color: "#80000000"
            }
        }
    ]
}
