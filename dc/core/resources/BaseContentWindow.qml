import QtQuick 1.1
import DisplayCluster 1.0
import "style.js" as Style

Rectangle {
    property alias titleBar: titleBar

    id: windowRect
    border.color: Style.windowBorderDefaultColor

    x: contentwindow.x
    y: contentwindow.y - (titleBar.visible ? titleBar.height : 0)
    width: contentwindow.width
    height: contentwindow.height + (titleBar.visible ? titleBar.height : 0)

    Rectangle
    {
        id: titleBar
        visible: displaygroup.showWindowTitles && contentwindow.label !== "Dock"
        width: parent.width
        height: Style.windowTitleHeight
        color: parent.border.color

        Text
        {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: Style.windowTitleFontSize / 4

            FontLoader { id: gothamBook; source: "qrc:/fonts/Gotham-Book.otf"; name: "qrc::gotham-book" }

            elide: Text.ElideRight
            text: contentwindow.label
            font { family: "qrc::gotham-book"; pixelSize: Style.windowTitleFontSize }
        }
    }

    states: [
        State {
            name: "focused"
            when: contentwindow.focused
            PropertyChanges {
                target: windowRect
                x: contentwindow.controller.getFocusedCoord().x
                y: contentwindow.controller.getFocusedCoord().y
                width: contentwindow.controller.getFocusedCoord().width
                height: contentwindow.controller.getFocusedCoord().height
                border.color: Style.windowBorderSelectedColor
            }
        },
        State {
            name: "selected"
            when: contentwindow.state === ContentWindow.SELECTED
            PropertyChanges {
                target: windowRect
                border.color: Style.windowBorderSelectedColor
            }
        },
        State {
            name: "moving"
            when: contentwindow.state === ContentWindow.MOVING
            PropertyChanges {
                target: windowRect
                border.color: Style.windowBorderMovingColor
            }
        },
        State {
            name: "resizing"
            when: contentwindow.state === ContentWindow.RESIZING
            PropertyChanges {
                target: windowRect
                border.color: Style.windowBorderResizingColor
            }
        },
        State {
            name: "hidden"
            when: contentwindow.state === ContentWindow.HIDDEN
            PropertyChanges {
                target: windowRect
                visible: false
            }
        }
    ]

    transitions: [
        Transition {
            to: "focused"
            reversible: true
            ColorAnimation {
                properties: "border.color"
                duration: Style.focusTransitionTime
            }
            NumberAnimation {
                properties: "x,y,height,width"
                duration: Style.focusTransitionTime
                easing.type: Easing.InOutQuad
            }
        }
    ]
}
