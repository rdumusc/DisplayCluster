import QtQuick 1.1
import DisplayCluster 1.0
import "style.js" as Style

Rectangle {
    border.width: Style.zoomContextBorderWidth
    border.color: Style.zoomContextBorderColor
    width: parent.width * 0.25
    height: parent.height * 0.25
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    anchors.bottomMargin: 0.25 * height
    anchors.leftMargin: anchors.bottomMargin
    visible: options.showZoomContext &&
             contentwindow.zoomRect.width !== 1 &&
             contentwindow.zoomRect.height !== 1

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
