import QtQuick 1.1
import DisplayCluster 1.0
import "style.js" as Style

Rectangle {
    property alias listview: buttons

    id: rootObj
    width: buttons.width + radius
    height: buttons.height
    color: Style.controlsColor
    border.color: Style.controlsBorderColor
    border.width: Style.controlsBorderWidth
    radius: Style.controlsRadius
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: parent.bottom
    anchors.bottomMargin: Style.controlsBottomMargin
    visible: contentwindow.label !== "Dock"
             && contentwindow.border === ContentWindow.NOBORDER
    opacity: contentwindow.controlsOpacity

    ListView {
        id: buttons
        width: (count + 2) * height + Math.max(count - 1, 0) * spacing
        height: Style.buttonsHeight
        anchors.horizontalCenter: parent.horizontalCenter
        orientation: ListView.Horizontal
        spacing: Style.buttonsSpacing
        delegate: WindowControlsDelegate {
        }
        header: FullscreenControlButton {
        }
        footer: CloseControlButton {
        }
        model: contentwindow.content.actions
    }
}
