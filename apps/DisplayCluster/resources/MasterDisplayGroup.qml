import QtQuick 2.0
import DisplayCluster 1.0
import "qrc:/qml/core/."
import "qrc:/qml/core/style.js" as Style

DisplayGroup {
    id: dispGroup
    showFocusContext: false

    MultitouchArea {
        anchors.fill: parent
        referenceItem: dispGroup
        z: controlPanel.z - 1
        onTapAndHold: view.backgroundTapAndHold(pos)
    }

    controlPanel.buttonDelegate: Component {
        ControlPanelDelegate {
            id: touchControlPanel
            MultitouchArea {
                anchors.fill: parent
                referenceItem: dispGroup

                onTapEnded: {
                    var action = touchControlPanel.ListView.view.model.get(index).action
                    var absPos = mapToItem(dispGroup, controlPanel.width
                                 + Style.panelsLeftOffset, 0)
                    var position = Qt.point(absPos.x, absPos.y)
                    cppcontrolpanel.processAction(action, position)
                }
            }
        }
    }
}
