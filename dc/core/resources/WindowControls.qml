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
             && contentwindow.border === ContentWindow.NOBORDER
    opacity: contentwindow.controlsOpacity

    ListView {
        id: buttons
        width: Style.buttonsSize
        height: (count + 3) * Style.buttonsSize
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
        delegate: WindowControlsDelegate {
        }
        model: contentwindow.content.actions
    }
}
