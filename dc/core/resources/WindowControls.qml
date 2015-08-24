import QtQuick 1.1
import DisplayCluster 1.0
import "style.js" as Style

Rectangle {
    property alias listview: buttons

    id: rootObj
    width: buttons.width + radius + (Style.buttonsSize - Style.buttonsImageSize)
    height: buttons.height + (Style.buttonsSize - Style.buttonsImageSize)
    color: Style.controlsDefaultColor
    border.color: Style.controlsDefaultColor
    border.width: Style.controlsBorderWidth
    radius: Style.controlsRadius
    anchors.right: parent.left
    anchors.top: parent.top
    anchors.topMargin: options.showWindowBorders ? -Style.windowBorderWidth / 2 : 0
    anchors.rightMargin: Style.controlsLeftMargin
    visible: opacity > 0 && contentwindow.label !== "Dock"
    opacity: 0

    ListView {
        id: buttons
        width: Style.buttonsSize
        property int fixed_buttons_count: 3
        height: (count + fixed_buttons_count) * Style.buttonsSize
        anchors.centerIn: parent
        orientation: ListView.Vertical
        header: fixedButtonsDelegate

        Component {
            id: fixedButtonsDelegate
            Column {
                CloseControlButton {
                }
                OneToOneControlButton {
                }
                FocusControlButton {
                }
            }
        }
        delegate: WindowControlsDelegate {
        }
        model: contentwindow.content.actions
    }

    states: [
        State {
            name: "focus_mode"
            when: contentwindow.focused
            extend: "selected"
            PropertyChanges {
                target: buttons
                fixed_buttons_count: 2
            }
        },
        State {
            name: "selected"
            when: contentwindow.selected
            extend: "opaque"
            PropertyChanges {
                target: rootObj
                color: Style.controlsFocusedColor
                border.color: Style.controlsFocusedColor
            }
        },
        State {
            name: "opaque"
            when: contentwindow.controlsVisible
                  && contentwindow.border === ContentWindow.NOBORDER
            PropertyChanges {
                target: rootObj
                opacity: 1
            }
        }
    ]

    transitions: [
        Transition {
            NumberAnimation {
                target: rootObj
                property: "opacity"
                duration: Style.focusTransitionTime
                easing.type: Easing.InOutQuad
            }
            ColorAnimation { duration: Style.focusTransitionTime }
            ColorAnimation { target: border; duration: Style.focusTransitionTime }
        }
    ]
}
