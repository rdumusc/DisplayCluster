import QtQuick 1.1
import DisplayCluster 1.0
import "style.js" as Style

Item {
    id: borderRect
    property int border: modelData
    state: "inactive"

    property bool isRight: border === ContentWindow.RIGHT
                           || border == ContentWindow.BOTTOM_RIGHT
                           || border === ContentWindow.TOP_RIGHT
    property bool isLeft: border === ContentWindow.LEFT
                          || border === ContentWindow.BOTTOM_LEFT
                          || border === ContentWindow.TOP_LEFT
    property bool isTop: border === ContentWindow.TOP
                         || border == ContentWindow.TOP_RIGHT
                         || border === ContentWindow.TOP_LEFT
    property bool isBottom: border === ContentWindow.BOTTOM
                            || border === ContentWindow.BOTTOM_LEFT
                            || border === ContentWindow.BOTTOM_RIGHT

    width: border === ContentWindow.TOP
           || border === ContentWindow.BOTTOM ? parent.width - Style.borderWidth
                                                + Style.windowBorderWidth / 2.0 : Style.borderWidth
    height: border === ContentWindow.RIGHT
            || border === ContentWindow.LEFT ? parent.height - Style.borderWidth
                                               + Style.windowBorderWidth / 2.0 : Style.borderWidth

    anchors.horizontalCenter: isRight ? parent.right : isLeft ? parent.left : parent.horizontalCenter
    anchors.verticalCenter: isTop ? parent.top : isBottom ? parent.bottom : parent.verticalCenter
    anchors.horizontalCenterOffset: isRight ? Style.windowBorderWidth
                                              / 4.0 : isLeft ? -Style.windowBorderWidth / 4.0 : 0
    anchors.verticalCenterOffset: isTop ? -Style.windowBorderWidth
                                          / 4.0 : isBottom ? Style.windowBorderWidth / 4.0 : 0

    Rectangle {
        id: controlCircle
        width: Style.borderWidth
        height: Style.borderWidth
        radius: Style.borderWidth
        color: Style.inactiveBorderColor
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

    states: [
        State {
            name: "active"
            when: contentwindow.border === border
            PropertyChanges {
                target: controlCircle
                color: Style.activeBorderColor
            }
        },
        State {
            name: "inactive"
            when: contentwindow.border === ContentWindow.NOBORDER
            PropertyChanges {
                target: controlCircle
                color: Style.inactiveBorderColor
            }
        }
    ]
}
