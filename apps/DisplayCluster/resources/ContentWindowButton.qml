import QtQuick 1.1

Image {
    property alias mousearea: mousearea

    width: 0.4 * Math.min(parent.width, parent.height)
    height: width
    sourceSize.width: 512
    sourceSize.height: 512
    MouseArea {
        id: mousearea
        anchors.fill: parent
        preventStealing: true
    }
}
