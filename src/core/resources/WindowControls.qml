import QtQuick 1.1
import DisplayCluster 1.0

Rectangle {
    property alias buttons: buttons.children
    width: 250
    height: 100
    color: "#80000000"
    border.color: "#a0000000"
    border.width: 3
    radius: 20
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.bottom: parent.bottom
    anchors.bottomMargin: 30

    Flow {
        id: buttons
        anchors.centerIn: parent
        anchors.margins: 4
        spacing: 15
        height: parent.height

        ControlButton {
            image: "qrc:///img/maximize.svg"
            objectName: "toggleFullscreenButton"
        }
        ControlButton {
            image: "qrc:///img/close.svg"
            objectName: "closeButton"
        }
    }
}
