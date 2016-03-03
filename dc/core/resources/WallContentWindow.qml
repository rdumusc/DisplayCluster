import QtQuick 2.0
import DisplayCluster 1.0
import "style.js" as Style

BaseContentWindow {
    id: windowRect

    // for contents with alpha channel such as SVG or PNG
    color: options.alphaBlending ? "transparent" : "black"

    property string imagesource: "image://texture/" + contentwindow.content.uri

    Item {
        id: contentItemArea
        anchors.bottom: parent.bottom
        anchors.bottomMargin: windowRect.border.width
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 2 * windowRect.border.width
        height: parent.height - windowRect.border.width - (titleBar.visible ? titleBar.height : windowRect.border.width)
        clip: true

        Item {
            id: contentItem
            objectName: "TilesParent"

            // Tiles bind to this signal from c++ to toggle borders visibility
            property bool showTilesBorder: options.showContentTiles
            // The auto-generated notifier does not emit the new value, do it
            signal showTilesBordersValueChanged(bool value)
            onShowTilesBorderChanged: showTilesBordersValueChanged(showTilesBorder)

            transform: [
                // Adjust tiles to content area
                Scale {
                    xScale: contentItemArea.width / (contentsync.tilesArea.width > 0 ? contentsync.tilesArea.width : contentwindow.content.size.width)
                    yScale: contentItemArea.height / (contentsync.tilesArea.height > 0 ? contentsync.tilesArea.height : contentwindow.content.size.height)
                },
                // Apply content zoom
                Translate {
                    x: -contentwindow.content.zoomRect.x * contentItemArea.width
                    y: -contentwindow.content.zoomRect.y * contentItemArea.height
                },
                Scale {
                    xScale: 1.0 / contentwindow.content.zoomRect.width
                    yScale: 1.0 / contentwindow.content.zoomRect.height
                }
            ]
        }
    }

    ZoomContext {
    }

    Text {
        id: statistics
        text: contentsync.statistics
        visible: options.showStatistics

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: Style.statisticsBorderMargin
        anchors.bottomMargin: Style.statisticsBorderMargin
        font.pointSize: Style.statisticsFontSize
        color: Style.statisticsFontColor
    }

    WindowControls {
    }

    ResizeCircles {
    }

    ParallelAnimation {
        id: openingAnimation
        NumberAnimation {
            target: windowRect
            property: "x"
            from: -contentwindow.width
            to: contentwindow.x
            duration: Style.panelsAnimationTime
            easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: windowRect
            property: "opacity"
            from: 0
            to: 1
            duration: Style.panelsAnimationTime
            easing.type: Easing.InOutQuad
        }
    }
    Component.onCompleted: if(contentwindow.isPanel) {openingAnimation.start()}
}
