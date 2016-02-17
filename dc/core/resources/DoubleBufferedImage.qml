import QtQuick 2.0

Item {
    property string source: ""
    property bool cache: true
    property bool showFront: true

    Image {
        anchors.fill: parent
        visible: parent.showFront
        source: parent.source
        cache: parent.cache
    }

    Image {
        anchors.fill: parent
        visible: !parent.showFront
        source: parent.source
        cache: parent.cache
    }
}
