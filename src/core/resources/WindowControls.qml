import QtQuick 1.1
import DisplayCluster 1.0

Rectangle {
    id: controls
    width: 300
    height: 100
    color: "#80000000"
    border.color: "#a0000000"
    border.width: 3
    radius: 20

    signal close()
    signal toggleFullscreen()

    Item {
        width: parent.width * 0.95 / 2
        height: parent.height * 0.95
        anchors.left: parent.left
        anchors.leftMargin: parent.radius
        anchors.verticalCenter: parent.verticalCenter
        Text {
            text: "max/min"
            color: "#a0000000"
            font.pixelSize: 32
            anchors.centerIn: parent
        }
        TouchArea {
            anchors.fill: parent
            onTap: controls.toggleFullscreen()
        }
    }
    Item {
        width: parent.width * 0.95 / 2
        height: parent.height * 0.95
        anchors.right: parent.right
        anchors.rightMargin: parent.radius
        anchors.verticalCenter: parent.verticalCenter
        Text {
            text: "close"
            color: "#a0000000"
            font.pixelSize: 32
            anchors.centerIn: parent
        }
        TouchArea {
            anchors.fill: parent
            onTap: controls.close()
        }
    }
}
