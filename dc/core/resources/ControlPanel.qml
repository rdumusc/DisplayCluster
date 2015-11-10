import QtQuick 1.1
import DisplayCluster 1.0
import "qrc:/qml/core/style.js" as Style

Rectangle {
    width: 780
    height: 920

    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left
    anchors.leftMargin: 50

    property alias buttons: buttons

    border.color: Style.controlsFocusedColor
    border.width: Style.borderWidth
    color: "transparent"

    ListView {
        header: Text {
            text: "CONTENT    "
            font.underline: true
            font.pixelSize: 70
            height: font.pixelSize + 30 // Add some margin below the text
        }
        id: buttons

        anchors.fill: parent
        anchors.margins: 50

        spacing: 20
        model: contentModel
        delegate: ControlPanelDelegate {
        }
    }

    ListModel {
        id: contentModel
        ListElement { label: "Add file"; icon: "qrc:///img/play.svg"; action: QmlControlPanel.OPEN_CONTENT }
        ListElement { label: "Start application"; icon: "qrc:///img/close.svg"; action: QmlControlPanel.OPEN_APPLICATION }
    }
}
