import QtQuick 1.1
import "style.js" as Style

Item {
    anchors.fill: parent

    property alias statistics: statistics.text

    Text {
        id: statistics
        text: (pixelstream !== null) ? pixelstream.statistics : "N/A" // context object pixelstream is set later
        visible: options.showStatistics

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: Style.statisticsBorderMargin
        anchors.bottomMargin: Style.statisticsBorderMargin
        font.pointSize: Style.statisticsFontSize
        color: Style.statisticsFontColor
    }
}
