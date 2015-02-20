import QtQuick 1.1
import DisplayCluster 1.0

Rectangle {
    anchors.fill: parent
    color: "transparent"

    property real baseOpacity: 0.4

    visible: contentwindow.label !== "Dock"
    opacity: contentwindow.controlsOpacity * baseOpacity

    signal borderPanned(variant delta, variant windowBorder)
    signal borderPanFinished

    TouchBorderRectangle {
        border: ContentWindow.TOP_LEFT
    }
    TouchBorderRectangle {
        border: ContentWindow.TOP
    }
    TouchBorderRectangle {
        border: ContentWindow.TOP_RIGHT
    }
    TouchBorderRectangle {
        border: ContentWindow.RIGHT
    }
    TouchBorderRectangle {
        border: ContentWindow.BOTTOM_RIGHT
    }
    TouchBorderRectangle {
        border: ContentWindow.BOTTOM
    }
    TouchBorderRectangle {
        border: ContentWindow.BOTTOM_LEFT
    }
    TouchBorderRectangle {
        border: ContentWindow.LEFT
    }
}
