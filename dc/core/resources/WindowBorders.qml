import QtQuick 2.0
import DisplayCluster 1.0
import "style.js" as Style

Rectangle {
    anchors.fill: parent
    color: "transparent"

    property real baseOpacity: Style.borderOpacity
    property alias borderDelegate: repeater.delegate

    visible: !contentwindow.isPanel
             && contentwindow.controlsVisible
             && contentwindow.state !== ContentWindow.SELECTED
    opacity: baseOpacity

    Repeater {
        id: repeater
        model: [ContentWindow.TOP_LEFT, ContentWindow.TOP, ContentWindow.TOP_RIGHT, ContentWindow.RIGHT, ContentWindow.BOTTOM_RIGHT, ContentWindow.BOTTOM, ContentWindow.BOTTOM_LEFT, ContentWindow.LEFT]
        delegate: BorderRectangle {
        }
    }
}
