import QtQuick 1.1
import DisplayCluster 1.0
import "style.js" as Style

BaseContentWindow {
    border.width: options.showWindowBorders ? Style.windowBorderWidth : 0
    // for contents with alpha channel such as SVG or PNG
    color: options.alphaBlending ? "transparent" : "black"

    ContentItem {
        objectName: "ContentItem"
        anchors.bottom: parent.bottom
        width: contentwindow.width
        height: contentwindow.height
    }

    ZoomContext {
    }

    WindowControls {
    }

    WindowBorders {
    }
}
