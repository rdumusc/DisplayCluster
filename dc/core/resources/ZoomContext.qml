import QtQuick 1.1
import DisplayCluster 1.0
import "style.js" as Style

Rectangle {
    function fuzzyCompare(value1, value2) {
        return Math.abs(value1 - value2) <= 0.0000001
    }

    function hasZoom(rect) {
        return !fuzzyCompare(rect.width, 1.0) || !fuzzyCompare(rect.height, 1.0)
    }

    border.width: Style.zoomContextBorderWidth
    border.color: Style.zoomContextBorderColor
    width: parent.width <= parent.height ? parent.width * 0.25 : parent.height * 0.25
    height: width / contentwindow.content.aspectRatio
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.bottomMargin: 0.25 * height
    anchors.leftMargin: anchors.bottomMargin
    visible: options.showZoomContext && hasZoom(contentwindow.zoomRect)

    ContentItem {
        objectName: "ZoomContextItem"
        anchors.fill: parent
        role: ContentItem.ROLE_PREVIEW

        Rectangle {
            border.width: Style.zoomContextSelectionWidth
            border.color: Style.zoomContextSelectionColor
            color: "transparent"
            x: contentwindow.zoomRect.x * parent.width
            y: contentwindow.zoomRect.y * parent.height
            width: parent.width * contentwindow.zoomRect.width
            height: parent.height * contentwindow.zoomRect.height
        }
    }
}
