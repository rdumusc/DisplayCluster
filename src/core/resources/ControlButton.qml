import QtQuick 1.1

Item {
    property alias image: image.source
    property string buttonId

    signal tapped(string buttonId)

    function sendTap() {
        tapped(buttonId)
    }

    width: height
    height: parent.height
    Image {
        id: image
        height: parent.height * 0.8
        width: height
        anchors.centerIn: parent
        // Force redraw the SVG
        sourceSize.width: width
        sourceSize.height: height
    }
}
