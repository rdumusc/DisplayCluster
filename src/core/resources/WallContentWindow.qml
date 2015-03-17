import QtQuick 1.1
import DisplayCluster 1.0
import "style.js" as Style

Rectangle {
    id: windowRect
    border.color: Style.windowBorderDefaultColor
    border.width: options.showWindowBorders ? Style.windowBorderWidth : 0

    x: contentwindow.x
    y: contentwindow.y
    width: contentwindow.width
    height: contentwindow.height

    ContentItem {
        objectName: "ContentItem"
        anchors.fill: parent
    }

    ZoomContext {
    }

    WindowControls {
    }

    WindowBorders {
    }

    states: [
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
}
