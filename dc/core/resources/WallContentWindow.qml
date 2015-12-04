import QtQuick 1.1
import DisplayCluster 1.0
import "style.js" as Style

BaseContentWindow {
    id: windowRect

    border.width: options.showWindowBorders && !isBackground ? Style.windowBorderWidth : 0
    // for contents with alpha channel such as SVG or PNG
    color: options.alphaBlending ? "transparent" : "black"

    ContentItem {
        property bool animating: parent.animating
        objectName: "ContentItem"
        anchors.bottom: parent.bottom
        width: parent.width
        height: parent.height - (titleBar.visible ? titleBar.height : 0)

        ZoomContext {
        }
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
