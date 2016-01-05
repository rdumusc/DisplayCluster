import QtQuick 2.0
import DisplayCluster 1.0
import "style.js" as Style

BaseContentWindow {
    id: windowRect

    // for contents with alpha channel such as SVG or PNG
    color: options.alphaBlending ? "transparent" : "black"

    property string imagesource: "image://" + contentwindow.content.sourceImage
                                 + contentsync.sourceParams

    Item {
        id: contentItemArea
        anchors.bottom: parent.bottom
        anchors.bottomMargin: windowRect.border.width
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 2 * windowRect.border.width
        height: parent.height - windowRect.border.width - (titleBar.visible ? titleBar.height : windowRect.border.width)

        Item {
            id: contentItem

            Repeater {
                model: contentsync.tiles

                DoubleBufferedImage {
                    x: model.modelData.coord.x
                    y: model.modelData.coord.y
                    width: model.modelData.coord.width > 0 ? model.modelData.coord.width : contentwindow.content.size.width
                    height: model.modelData.coord.height > 0 ? model.modelData.coord.height : contentwindow.content.size.height

                    property string tileIndex: model.modelData.index >= 0 ? "?" + model.modelData.index : ""
                    source: imagesource + tileIndex

                    cache: contentsync.allowsTextureCaching

                    Rectangle {
                        visible: options.showContentTiles
                        anchors.fill: parent
                        border.color: Style.segmentBorderColor
                        color: "transparent"
                    }
                }
            }
            transform: Scale {
                xScale: contentItemArea.width / contentwindow.content.size.width
                yScale: contentItemArea.height / contentwindow.content.size.height
            }
        }
    }

    ZoomContext {
        id: zoomContext
        image.source: visible ? imagesource : ""
        image.cache: contentsync.allowsTextureCaching
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

    WindowBorders {
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
