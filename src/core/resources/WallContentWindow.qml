import QtQuick 1.1
import DisplayCluster 1.0
import "style.js" as Style

BaseContentWindow {
    border.width: options.showWindowBorders ? Style.windowBorderWidth : 0
    color: "transparent" // for contents with alpha channel such as SVG or PNG

    ContentItem {
        objectName: "ContentItem"
        anchors.fill: parent
    }

    ZoomContext {
    }

    WindowControls {
    }

    WindowBorders {
    }
}
