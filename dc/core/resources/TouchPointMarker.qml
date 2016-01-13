import QtQuick 2.0
import DisplayCluster 1.0
import "style.js" as Style

Rectangle {
    x: xposition - Style.touchPointMarkerSize / 2.0
    y: yposition - Style.touchPointMarkerSize / 2.0
    z: Style.overlayZ
    width: Style.touchPointMarkerSize
    height: Style.touchPointMarkerSize
    radius: Style.touchPointMarkerSize
    color: Style.touchPointMarkerCenterColor
    border.width: Style.touchPointMarkerBorderSize
    border.color: Style.touchPointMarkerBorderColor
    visible: options.showTouchPoints
}