import QtQuick 2.0

Rectangle {
    anchors.fill: parent
    color: "blue"
    objectName: "background"

    Rectangle {
        width: parent.width * 0.5
        height: parent.height * 0.5
        anchors.centerIn: parent
        color: "pink"
    }
}
