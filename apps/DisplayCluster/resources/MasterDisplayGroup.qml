import QtQuick 1.1
import DisplayCluster 1.0
import DisplayClusterApp 1.0
import "qrc:/qml/core/."

DisplayGroup {
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
                    var position = Qt.point(controlPanel.width,
                                            displaygroup.height / 2)
                    cppcontrolpanel.processAction(action, position)
                }
            }
        }
    }
}
