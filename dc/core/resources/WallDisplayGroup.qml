import QtQuick 2.0
import DisplayCluster 1.0
import "qrc:/clock/."
import "qrc:/qml/core/style.js" as Style

DisplayGroup {
    Clock {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: 0.5 * width
        anchors.rightMargin: 0.5 * width
        visible: options.showClock
        displayedHeight: displaygroup.height * Style.clockScale
    }
}
