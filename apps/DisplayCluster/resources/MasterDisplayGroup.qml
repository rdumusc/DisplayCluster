import QtQuick 1.1
import DisplayCluster 1.0
import DisplayClusterApp 1.0
import "qrc:/qml/core/."

DisplayGroup {
    TouchArea {
        anchors.fill: parent
        onTap: {
            dggv.onBackgroundTap(position)
        }
        onTapAndHold: {
            dggv.onBackgroundTapAndHold(position)
        }
    }
}
