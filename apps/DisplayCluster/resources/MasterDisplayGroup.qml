import QtQuick 2.0
import DisplayCluster 1.0
import DisplayClusterApp 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

DisplayGroup {
    id: dispGroup
    showFocusContext: false
    layer.enabled: true
    layer.mipmap: true
    layer.smooth: true

    property int numberOfTilesX: 1
    property int numberOfTilesY: 1
    property int mullionWidth: 0

    property int screenWidth: (displaygroup.width - ((numberOfTilesX - 1)
                              * mullionWidth)) / numberOfTilesX
    property int screenHeight: (displaygroup.height - ((numberOfTilesY - 1)
                               * mullionWidth)) / numberOfTilesY

    property bool higherAspectRatio: (view.width / view.height) >
                                     (displaygroup.width / displaygroup.height)

    property int marginWidth: higherAspectRatio ? 0 : (displaygroup.width
                              * Style.masterWindowMarginFactor)
    property int marginHeight: higherAspectRatio ? (displaygroup.height
                               * Style.masterWindowMarginFactor): 0

    property int offsetX: higherAspectRatio ? ( view.width - displaygroup.width
                          * scale ) / 2.0 : displaygroup.width
                          * Style.masterWindowMarginFactor * scale / 2.0
    property int offsetY: higherAspectRatio ? displaygroup.height
                          * Style.masterWindowMarginFactor* scale / 2.0
                          : ( view.height - displaygroup.height * scale ) / 2.0

    property int totalWidth: displaygroup.width + (2 * mullionWidth)
                             + marginWidth
    property int totalHeight: displaygroup.height + (2 * mullionWidth)
                              + marginHeight

    property real scale: higherAspectRatio ? (view.height / totalHeight)
                                           : (view.width / totalWidth)

    width: displaygroup.width + 2 * mullionWidth
    height: displaygroup.height + 2 * mullionWidth

    TouchArea {
        z: controlPanel.z - 1
        anchors.fill: parent
        onTap: {
            view.notifyBackgroundTap(position)
        }
        onTapAndHold: {
            view.notifyBackgroundTapAndHold(position)
        }
    }

    controlPanel.buttonDelegate: Component {
        ControlPanelDelegate {
            id: touchControlPanel
            TouchMouseArea {
                anchors.fill: parent
                onTap: {
                    var action = touchControlPanel.ListView.view.model.get(index).action
                    var absPos = mapToItem(dispGroup, controlPanel.width
                                 + Style.panelsLeftOffset, 0)
                    var position = Qt.point(absPos.x, absPos.y)
                    cppcontrolpanel.processAction(action, position)
                }
            }
        }
    }

    Rectangle {
        color: Style.masterWindowBezelsColor
        width: displaygroup.width + 2 * mullionWidth
        height: displaygroup.height + 2 * mullionWidth
    }

    transform: [
        Scale {
            xScale: scale
            yScale: scale
        },
        Translate {
            x : offsetX
            y : offsetY
        }
    ]

    Grid
    {
        x: mullionWidth
        y: mullionWidth
        columns: numberOfTilesX
        rows: numberOfTilesY
        spacing: mullionWidth
        Repeater {
            layer.mipmap: true
            id: screens
            model: numberOfTilesX * numberOfTilesY
            delegate: Rectangle {
                //Generalized checker pattern computation
                color: ((index%2 == 0) ^
                       (!(Math.floor(index / numberOfTilesX)%2 == 0)
                       && (numberOfTilesX%2 == 0))) ?
                       Style.masterWindowFirstCheckerColor
                       : Style.masterWindowSecondCheckerColor
                width: screenWidth
                height: screenHeight
            }
        }
    }
}
