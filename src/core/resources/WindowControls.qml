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
        anchors.leftMargin: parent.width * 0.05
        anchors.verticalCenter: parent.verticalCenter
        Image {
            source: "qrc:///img/maximize.svg"
            height: parent.height * 0.8
            width: height
            anchors.centerIn: parent
            // Force redraw the SVG
            sourceSize.width: width
            sourceSize.height: height
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
        anchors.rightMargin: parent.width * 0.05
        anchors.verticalCenter: parent.verticalCenter
        Image {
            source: "qrc:///img/close.svg"
            height: parent.height * 0.8
            width: height
            anchors.centerIn: parent
            // Force redraw the SVG
            sourceSize.width: width
            sourceSize.height: height
        }
        TouchArea {
            anchors.fill: parent
            onTap: controls.close()
        }
    }
}
