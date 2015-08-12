import QtQuick 1.1
import DisplayCluster 1.0
import "style.js" as Style

Rectangle {
    property alias listview: buttons

    id: rootObj
    width: buttons.width + radius + (Style.buttonsSize - Style.buttonsImageSize)
    height: buttons.height + (Style.buttonsSize - Style.buttonsImageSize)
    color: Style.controlsColor
    border.color: Style.controlsBorderColor
    border.width: Style.controlsBorderWidth
    radius: Style.controlsRadius
    anchors.right: parent.left
    anchors.top: parent.top
    anchors.topMargin: options.showWindowBorders ? -Style.windowBorderWidth / 2 : 0
    anchors.rightMargin: Style.controlsLeftMargin
    visible: contentwindow.label !== "Dock"
             && contentwindow.controlsVisible
             && contentwindow.border === ContentWindow.NOBORDER

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
                FullscreenControlButton {
                }
            }
        }
        Component {
            id: fixedButtonsFocusDelegate
            Column {
                id: column
                CloseControlButton {
                }
                FullscreenControlButton {
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
            PropertyChanges {
                target: buttons
                fixed_buttons_count: 2
            }
            PropertyChanges {
                target: rootObj
                visible: true
            }
        }
    ]
}
