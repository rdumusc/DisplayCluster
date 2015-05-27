import QtQuick 1.1
import DisplayCluster 1.0
import "style.js" as Style

BaseContentWindow {
    border.width: options.showWindowBorders ? Style.windowBorderWidth : 0
    // for contents with alpha channel such as SVG or PNG
    color: options.alphaBlending ? "transparent" : "black"

    ContentItem {
        objectName: "ContentItem"
        anchors.fill: parent
    }

    Rectangle
    {
        visible: displaygroup.showWindowTitles && contentwindow.label !== "Dock"
        y: -Style.windowTitleHeight + ( parent.border.width / 2 )
        x: 0
        width: parent.width
        height: Style.windowTitleHeight - parent.border.width
        color: parent.border.color
        border.width: parent.border.width
        border.color: parent.border.color

        Text
        {
            y: Style.windowTitleHeight / 8
            x: Style.windowTitleFontSize / 4
            FontLoader { id: gothamBook; source: "qrc:/fonts/Gotham-Book.otf"; name: "qrc::gotham-book" }

            width: parent.width
            elide: Text.ElideRight
            text: contentwindow.label
            font { family: "qrc::gotham-book"; pixelSize: Style.windowTitleFontSize }
        }
    }

    ZoomContext {
    }

    WindowControls {
    }

    WindowBorders {
    }
}
