import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    Layout.fillWidth: true
    Layout.fillHeight: true
    color: "#000000"

    property int currentStep: 0
    property var videoData: ({})

    // 组件定义
    Component {
        id: captureUploadComponent

        CaptureUploadPage {
            onVideoSelected: function(filePath) {
                videoData.path = filePath
                videoData.thumbnail = ""
                currentStep = 1
            }
        }
    }

    Component {
        id: editComponent

        EditPage {
            videoSource: videoData.path
            onEditComplete: function(musicData) {
                videoData.music = musicData
                currentStep = 2
            }
            onBackRequested: currentStep = 0
        }
    }

    Component {
        id: publishComponent

        PublishPage {
            onPublishComplete: {
                console.log("发布完成")
            }
            onBackRequested: currentStep = 1
        }
    }

    // 使用Loader动态加载组件
    Loader {
        id: pageLoader
        anchors.fill: parent

        sourceComponent: {
            if (currentStep === 0)
                return captureUploadComponent
            else if (currentStep === 1)
                return editComponent
            else if (currentStep === 2)
                return publishComponent
            else
                return captureUploadComponent
        }
    }
}
