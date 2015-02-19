import QtQuick 1.1
import DisplayCluster 1.0

Rectangle {
    property alias listview: buttons

    id: rootObj
    width: buttons.width + radius
    height: buttons.height
    color: "#80000000"
    border.color: "#a0000000"
    border.width: 3
    radius: 20
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 120
    visible: contentwindow.label !== "Dock"
             && contentwindow.border === ContentWindow.NOBORDER
    opacity: contentwindow.controlsOpacity

    ListView {
        id: buttons
        width: (count + 2) * height + Math.max(count - 1, 0) * spacing
        height: 100
        anchors.horizontalCenter: parent.horizontalCenter
        orientation: ListView.Horizontal
        spacing: 10
        delegate: WindowControlsDelegate {
        }
        header: FullscreenControlButton {
        }
        footer: CloseControlButton {
        }
        model: contentwindow.content.actions
    }
}
