import QtQuick 1.1
import "style.js" as Style

Item {
    anchors.fill: parent

    property alias statistics: statistics.text

    Repeater {
        // context object pixelstream is set later
        model: pixelstream !== null ? pixelstream.segments : undefined
        Rectangle {
            visible: options.showStreamingSegments
            width: model.modelData.coord.width
            height: model.modelData.coord.height
            x: model.modelData.coord.x
            y: model.modelData.coord.y
            border.color: Style.segmentBorderColor
            color: "transparent"
        }
    }

    Text {
        id: statistics
        text: pixelstream !== null ? pixelstream.statistics : "N/A"
        visible: options.showStatistics

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: Style.statisticsBorderMargin
        anchors.bottomMargin: Style.statisticsBorderMargin
        font.pointSize: Style.statisticsFontSize
        color: Style.statisticsFontColor
    }
}
