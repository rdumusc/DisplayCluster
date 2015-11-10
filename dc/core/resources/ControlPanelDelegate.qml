import QtQuick 1.1


Row {
    id: button
    width: parent.width
    height: 70

    spacing: 20
    Image {
        source: icon
        height: button.height
        width: height
    }
    Text {
        text: label
        font.pixelSize: button.height * 0.8 // Fill the button vertically
    }
}
