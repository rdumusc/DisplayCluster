import QtQuick 1.1
import DisplayCluster 1.0
import DisplayClusterApp 1.0
import "qrc:/qml/core/."

BorderRectangle {
    TouchArea {
        anchors.fill: parent
        onPan: parent.parent.borderPanned(delta, parent.border)
        onPanFinished: parent.parent.borderPanFinished()
    }
}
