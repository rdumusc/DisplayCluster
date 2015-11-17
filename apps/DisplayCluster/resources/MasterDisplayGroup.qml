import QtQuick 1.1
import DisplayCluster 1.0
import DisplayClusterApp 1.0
import "qrc:/qml/core/."

DisplayGroup {
    id: dispGroup
    showFocusContext: false
    TouchArea {
        anchors.fill: parent
        onTap: {
            dggv.notifyBackgroundTap(position)
        }
        onTapAndHold: {
            dggv.notifyBackgroundTapAndHold(position)
        }
    }

    controlPanel.buttonDelegate: Component {
        ControlPanelDelegate {
            id: touchControlPanel
            TouchMouseArea {
                anchors.fill: parent
                onTap: {
                    var action = touchControlPanel.ListView.view.model.get(index).action
                    var absPos = mapToItem(dispGroup, width, height/2)
                    var position = Qt.point(absPos.x, absPos.y)
                    cppcontrolpanel.processAction(action, position)
                }
            }
        }
    }
}
