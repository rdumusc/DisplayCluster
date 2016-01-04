import QtQuick 2.0

Item {
    property string source: ""
    property bool loading: image1.status === Image.Loading ||
                           image2.status === Image.Loading

    property bool cache: true

    onSourceChanged: {
        if(image1.visible)
            image2.source = source
        else
            image1.source = source
    }

    function swapBuffers() {
        image1.visible = !image1.visible
    }

    Image {
        id: image1
        objectName: "image1"
        anchors.fill: parent

        asynchronous: true
        onStatusChanged: if(status === Image.Ready ) { swapBuffers(); }

        cache: parent.cache
    }

    Image {
        id: image2
        objectName: "image2"
        anchors.fill: parent

        visible: !image1.visible

        asynchronous: true
        onStatusChanged: if(status === Image.Ready ) { swapBuffers(); }

        cache: parent.cache
    }
}
