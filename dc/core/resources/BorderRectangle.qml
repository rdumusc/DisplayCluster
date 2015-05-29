import QtQuick 1.1
import DisplayCluster 1.0
import "style.js" as Style

Rectangle {
    id: borderRect
    property int borderSize: Style.borderWidth
    property int border: modelData
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
                color: Style.activeBorderColor
            }
        },
        State {
            name: "inactive"
            when: contentwindow.border === ContentWindow.NOBORDER
            PropertyChanges {
                target: borderRect
                color: Style.inactiveBorderColor
            }
        }
    ]
}
