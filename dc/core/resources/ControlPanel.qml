import QtQuick 1.1
import DisplayCluster 1.0
import "qrc:/qml/core/style.js" as Style

Rectangle {
    width: Style.controlPanelWidth
    height: Style.controlPanelHeight

    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left
    anchors.leftMargin: Style.controlPanelPadding

    property Component buttonDelegate: ControlPanelDelegate {
    }

    border.color: Style.controlsFocusedColor
    border.width: Style.borderWidth
    color: Style.controlPanelBackground

    ListView {
        anchors.fill: parent
        anchors.margins: Style.controlPanelPadding
        spacing: Style.controlPanelPadding
        interactive: false // Don't let users scroll the list

        model: ListModel {
            ListElement {
                title: "CONTENT    "
                subItems: [
                    ListElement {
                        label: "Add file"
                        icon: "qrc:///img/play.svg"
                        action: QmlControlPanel.OPEN_CONTENT
                    },
                    ListElement {
                        label: "Start application"
                        icon: "qrc:///img/close.svg"
                        action: QmlControlPanel.OPEN_APPLICATION
                    }
                ]
            }
            ListElement {
                title: "SESSION    "
                subItems: [
                    ListElement {
                        label: "New"
                        icon: "qrc:///img/play.svg"
                        action: QmlControlPanel.NEW_SESSION
                    },
                    ListElement {
                        label: "Open"
                        icon: "qrc:///img/close.svg"
                        action: QmlControlPanel.LOAD_SESSION
                    }
                ]
            }
        }
        delegate: sectionDelegate
    }

    Component {
        id: sectionDelegate

        ListView {
            width: parent.width
            height: Style.controlPanelTextSize * count + headerItemHeight
            spacing: Style.controlPanelTextSpacing
            interactive: false // Don't let users scroll the list
            // workaround for QtQuick1 which is missing the headerItem property
            property real headerItemHeight: Style.controlPanelTextSize
                                            + Style.controlPanelTextSpacing

            model: subItems
            delegate: buttonDelegate
            header: Text {
                text: title
                font.underline: true
                font.pixelSize: Style.controlPanelTextSize
                height: font.pixelSize + Style.controlPanelTextSpacing
            }
        }
    }
}
