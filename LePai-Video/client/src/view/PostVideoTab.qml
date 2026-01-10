import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    id: rootContainer  // 给外层容器一个id
    Layout.fillWidth: true
    Layout.fillHeight: true
    color: "#000000"

    property int currentStep: 0
    property var videoData: ({
        path: "",          // 原视频路径
        fullVideoData: {}  // 完整的视频信息
    })

    // 组件定义
    Component {
        id: captureUploadComponent

        CaptureUploadPage {
            onVideoSelected: function(filePath) {
                rootContainer.videoData.path = filePath
                rootContainer.videoData.fullVideoData = {}
                currentStep = 1
                console.log("选择视频:", filePath)
            }
        }
    }

    Component {
        id: editComponent

        EditPage {
            videoSource: rootContainer.videoData.path
            onEditComplete: function(fullVideoData) {
                rootContainer.videoData.fullVideoData = fullVideoData || {}
                console.log("保存编辑数据:", JSON.stringify(rootContainer.videoData.fullVideoData, null, 2))
                currentStep = 2
            }
            onBackRequested: currentStep = 0
        }
    }

    Component {
        id: publishComponent

        PublishPage {
            // 使用根容器的数据
            videoData: rootContainer.videoData.fullVideoData || {}

            onPublishComplete: {
                console.log("发布完成")
                rootContainer.videoData.fullVideoData = {}
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

    // 调试：监控步骤切换
    onCurrentStepChanged: {
        console.log("=== 页面切换 ===")
        console.log("当前步骤:", currentStep)

        if (currentStep === 1) {
            console.log("进入编辑页面，视频路径:", videoData.path)
        } else if (currentStep === 2) {
            console.log("进入发布页面，数据:", JSON.stringify(videoData.fullVideoData, null, 2))

            if (!videoData.fullVideoData || Object.keys(videoData.fullVideoData).length === 0) {
                console.error("错误：发布页面没有接收到数据！")
            }
        }
    }
}
