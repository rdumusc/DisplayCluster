import QtQuick 2.0
import DisplayCluster 1.0

ControlButton {
    image: "qrc:///img/oneToOne.svg"
    visible: !contentwindow.isPanel && !contentwindow.focused
}
