import QtQuick 1.1
import DisplayCluster 1.0

Rectangle {
    border.width: 10
    border.color: "white"
    width: parent.width * 0.25
    height: parent.height * 0.25
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.bottomMargin: 0.25 * height
    anchors.leftMargin: 0.25 * width
    visible: options.showZoomContext &&
             contentwindow.zoomRect.width !== 1 &&
             contentwindow.zoomRect.height !== 1

    ZoomContextItem {
        objectName: "ZoomContextItem"
        anchors.fill: parent

        Rectangle {
            border.width: 5
            border.color: "red"
            color: "transparent"
            x: contentwindow.zoomRect.x * parent.width
            y: contentwindow.zoomRect.y * parent.height
            width: parent.width * contentwindow.zoomRect.width
            height: parent.height * contentwindow.zoomRect.height
        }
    }
}
