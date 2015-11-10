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

    controlPanel.buttons.delegate: touchControlPanelDelegate

    Component {
        id: touchControlPanelDelegate
        ControlPanelDelegate {
            TouchMouseArea {
                anchors.fill: parent
                onTap: {
                    var action = controlPanel.buttons.model.get(index).action
                    var position = Qt.point(controlPanel.width,
                                            displaygroup.height / 2)
                    cppcontrolpanel.processAction(action, position)
                }
            }
        }
    }
}
