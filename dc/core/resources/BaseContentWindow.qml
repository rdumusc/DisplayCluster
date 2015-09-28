import QtQuick 1.1
import DisplayCluster 1.0
import "style.js" as Style

Rectangle {
    id: windowRect

    property alias titleBar: titleBar
    property int stackingOrder: 0
    property bool isBackground: false
    property bool animating: false
    property real heightOffset: titleBar.visible ? titleBar.height : 0
    property real yOffset: 0.5 * heightOffset

    x: contentwindow.x
    y: contentwindow.y - yOffset
    z: stackingOrder
    width: contentwindow.width
    height: contentwindow.height + heightOffset
    border.color: Style.windowBorderDefaultColor

    Rectangle
    {
        id: titleBar
        visible: displaygroup.showWindowTitles && !windowRect.isBackground && contentwindow.label !== "Dock"
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
                y: contentwindow.controller.getFocusedCoord().y - yOffset
                width: contentwindow.controller.getFocusedCoord().width
                height: contentwindow.controller.getFocusedCoord().height + heightOffset
                border.color: Style.windowBorderSelectedColor
                z: stackingOrder + Style.focusZorder
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

    function startAnimating() { animating = state == "focused" }
    function stopAnimating() { animating = state != "focused" }

    transitions: [
        Transition {
            to: "focused"
            reversible: true
            // onRunningChanged does not work, only work around is to change
            // animating property at the beginning and end of a sequence.
            SequentialAnimation {
                ScriptAction { script: startAnimating(); }
                PropertyAction { property: "z" } // Immediately set the z value
                NumberAnimation {
                    properties: "x,y,height,width"
                    duration: Style.focusTransitionTime
                    easing.type: Easing.InOutQuad
                }
                ScriptAction { script: stopAnimating(); }
            }
            ColorAnimation {
                properties: "border.color"
                duration: Style.focusTransitionTime
            }
        }
    ]
}
