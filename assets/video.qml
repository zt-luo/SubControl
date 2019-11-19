import QtQuick 2.0
import QtQuick.Controls 1.0

import org.freedesktop.gstreamer.GLVideoItem 1.0


Rectangle {
    objectName:"rectangle"
    Item {
        anchors.fill: parent

        GstGLVideoItem {
            id: video
            objectName: "videoItem"
            anchors.centerIn: parent
            width: parent.width
            height: parent.height
        }
    }


    // Button to start/stop video recording
    property bool _running: false

    Item {
        anchors.bottom:     parent.bottom
        anchors.bottomMargin: 8
        anchors.right:      parent.right
        anchors.rightMargin:  8
        height:             18 * 2
        width:              height
        visible:            true
        Rectangle {
            id:                 recordBtnBackground
            anchors.top:        parent.top
            anchors.bottom:     parent.bottom
            width:              height
            radius:             height
            color:              "gray"
            SequentialAnimation on opacity {
                running:        _running
                loops:          Animation.Infinite
                PropertyAnimation { to: 0.5; duration: 500 }
                PropertyAnimation { to: 1.0; duration: 500 }
            }
        }

        Image {
            anchors.top:                parent.top
            anchors.bottom:             parent.bottom
            anchors.horizontalCenter:   parent.horizontalCenter
            width:                      height * 0.625
            sourceSize.width:           width
            source:                     "icon/camera.svg"
            visible:                    recordBtnBackground.visible
            fillMode:                   Image.PreserveAspectFit
        }

        Connections
        {
            //qml 连接 c++ 信号
            target:videoReceiver
            onRecordingChanged:
            {
                if (videoReceiver.recording) {
                    recordBtnBackground.color = "red"
                    _running = true
                    recordBtnBackground.opacity = 1

                } else {
                    recordBtnBackground.color = "gray"
                    _running = false
                    recordBtnBackground.opacity = 1
                }
            }
        }

        Connections
        {
            //qml 连接 c++ 信号
            target:videoReceiver
            onMouseOverRecordingButtonChanged:
            {
                if (videoReceiver.mouseOverRecordingButton) {
                    if (_running)
                    {
                        recordBtnBackground.color = "darkred"
                    }
                    else
                    {
                        recordBtnBackground.color = "darkgray"
                    }

                    recordBtnBackground.opacity = 1

                } else {
                    if (_running)
                    {
                        recordBtnBackground.color = "red"
                    }
                    else
                    {
                        recordBtnBackground.color = "gray"
                    }

                    recordBtnBackground.opacity = 1
                }
            }
        }
    }
}
