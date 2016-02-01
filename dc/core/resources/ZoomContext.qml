import QtQuick 2.0
import DisplayCluster 1.0
import "style.js" as Style

Rectangle {
    function fuzzyCompare(value1, value2) {
        return Math.abs(value1 - value2) <= 0.0000001
    }

    function hasZoom(rect) {
        return !fuzzyCompare(rect.width, 1.0) || !fuzzyCompare(rect.height, 1.0)
    }

    property alias image: image

    border.width: Style.zoomContextBorderWidth
    border.color: Style.zoomContextBorderColor
    width: parent.width <= parent.height ? parent.width * 0.25 : parent.height * 0.25
    height: width / contentwindow.content.aspectRatio
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.bottomMargin: 0.25 * height
    anchors.leftMargin: anchors.bottomMargin
    visible: options.showZoomContext && hasZoom(contentwindow.content.zoomRect)

    Image {
        id: image
        objectName: "ZoomContextItem"
        anchors.fill: parent

        Rectangle {
            border.width: Style.zoomContextSelectionWidth
            border.color: Style.zoomContextSelectionColor
            color: "transparent"
            x: contentwindow.content.zoomRect.x * parent.width
            y: contentwindow.content.zoomRect.y * parent.height
            width: parent.width * contentwindow.content.zoomRect.width
            height: parent.height * contentwindow.content.zoomRect.height
        }
    }
}
