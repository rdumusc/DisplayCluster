import QtQuick 1.1

Image {
    property alias mousearea: mousearea

    width: 0.4 * Math.min(parent.width, parent.height)
    height: width
    // Force redraw the SVG
    sourceSize.width: width
    sourceSize.height: height
    MouseArea {
        id: mousearea
        anchors.fill: parent
        preventStealing: true
    }
}
