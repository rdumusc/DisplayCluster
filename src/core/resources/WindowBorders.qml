import QtQuick 1.1
import DisplayCluster 1.0

Rectangle {
    anchors.fill: parent
    color: "transparent"

    property real baseOpacity: 0.4
    property alias borderDelegate: repeater.delegate

    visible: contentwindow.label !== "Dock"
             && contentwindow.state !== ContentWindow.SELECTED
    opacity: contentwindow.controlsOpacity * baseOpacity

    Repeater {
        id: repeater
        model: [ContentWindow.TOP_LEFT, ContentWindow.TOP, ContentWindow.TOP_RIGHT, ContentWindow.RIGHT, ContentWindow.BOTTOM_RIGHT, ContentWindow.BOTTOM, ContentWindow.BOTTOM_LEFT, ContentWindow.LEFT]
        delegate: BorderRectangle {
        }
    }
}
