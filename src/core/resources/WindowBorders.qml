import QtQuick 1.1
import DisplayCluster 1.0

Rectangle {
    anchors.fill: parent
    color: "transparent"

    property real baseOpacity: 0.4

    visible: contentwindow.label !== "Dock"
             && contentwindow.state !== ContentWindow.SELECTED
    opacity: contentwindow.controlsOpacity * baseOpacity

    BorderRectangle {
        border: ContentWindow.TOP_LEFT
    }
    BorderRectangle {
        border: ContentWindow.TOP
    }
    BorderRectangle {
        border: ContentWindow.TOP_RIGHT
    }
    BorderRectangle {
        border: ContentWindow.RIGHT
    }
    BorderRectangle {
        border: ContentWindow.BOTTOM_RIGHT
    }
    BorderRectangle {
        border: ContentWindow.BOTTOM
    }
    BorderRectangle {
        border: ContentWindow.BOTTOM_LEFT
    }
    BorderRectangle {
        border: ContentWindow.LEFT
    }
}
