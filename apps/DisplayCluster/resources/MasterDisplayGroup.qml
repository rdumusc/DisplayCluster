import QtQuick 1.1
import DisplayCluster 1.0
import DisplayClusterApp 1.0
import "qrc:/qml/core/."

DisplayGroup {
    showFocus: false
    TouchArea {
        anchors.fill: parent
        onTap: {
            dggv.notifyBackgroundTap(position)
        }
        onTapAndHold: {
            dggv.notifyBackgroundTapAndHold(position)
        }
    }
}
