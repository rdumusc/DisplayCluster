import QtQuick 2.0
import DisplayCluster 1.0
import "style.js" as Style

BaseContentWindow {
    id: windowRect

    border.width: options.showWindowBorders && !isBackground ? Style.windowBorderWidth : 0
    // for contents with alpha channel such as SVG or PNG
    color: options.alphaBlending ? "transparent" : "black"

    property string imagesource: "image://" + contentwindow.content.sourceImage
                                 + contentsync.sourceParams

    Item {
        id: contentItemArea
        anchors.bottom: parent.bottom
        width: parent.width
        height: parent.height - (titleBar.visible ? titleBar.height : 0)

        Item {
            id: contentItem
            anchors.fill: parent

            Repeater {
                model: contentsync.tiles

                DoubleBufferedImage {
                    x: model.modelData.coord.x
                    y: model.modelData.coord.y
                    width: model.modelData.coord.width > 0 ? model.modelData.coord.width : parent.width
                    height: model.modelData.coord.height > 0 ? model.modelData.coord.height : parent.height

                    property string tileIndex: model.modelData.index >= 0 ? "?" + model.modelData.index : ""
                    source: imagesource + tileIndex

                    cache: contentsync.allowsTextureCaching
                }
            }
        }
    }

    ZoomContext {
        id: zoomContext
        image.source: visible ? imagesource : ""
        image.cache: contentsync.allowsTextureCaching
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
