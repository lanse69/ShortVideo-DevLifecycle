import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#000000"

    // 接收从编辑页面传来的数据
    property var videoData: ({})

    signal publishComplete
    signal backRequested

    // 组件初始化时检查数据
    Component.onCompleted: {
        console.log("发布页面收到视频数据:")
        console.log("- 视频路径:", videoData.videoPath || "无")
        console.log("- 原视频路径:", videoData.originalVideoPath || "无")
        console.log("- 临时文件路径:", videoData.tempFilePath || "无")
        console.log("- 是否合并过:", videoData.hasMerged || false)
        console.log("- 是否有音乐:", videoData.hasMusic || false)
    }

    Column {
        anchors.fill: parent
        anchors.margins: Math.min(parent.width * 0.05, 20)
        spacing: Math.min(parent.width * 0.05, 10)

        // 视频信息显示区域
        Rectangle {
            width: parent.width
            height: 60
            color: "#222222"
            radius: 8
            visible: videoData.videoPath

            Column {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5

                // 视频文件信息
                Text {
                    width: parent.width
                    text: "视频文件: " + (fileutils ? fileutils.getFileName(videoData.videoPath || "") : videoData.videoPath || "无")
                    color: "#FF2C5C"
                    font.pixelSize: 14
                    elide: Text.ElideMiddle
                }

                // 视频状态信息
                Text {
                    width: parent.width
                    text: {
                        if (videoData.hasMerged) return "状态: 已合并音频"
                        if (videoData.hasMusic) return "状态: 已选音乐（未合并）"
                        return "状态: 原视频"
                    }
                    color: "#50c878"
                    font.pixelSize: 12
                }
            }
        }

        // 描述输入区域
        TextField {
            id: descriptionArea
            width: parent.width
            height: parent.height * 0.6
            placeholderText: "添加视频描述..."
            placeholderTextColor: "#888888"
            color: "white"
            font.pixelSize: Math.max(14, parent.width * 0.04)
            background: Rectangle {
                color: "#222222"
                radius: Math.min(parent.width * 0.02, 8)
            }
            padding: Math.min(parent.width * 0.03, 10)
            leftPadding: Math.min(parent.width * 0.03, 10)
        }

        // 底部按钮区域
        Row {
            width: parent.width
            height: Math.min(parent.width * 0.12, 50)
            spacing: Math.min(parent.width * 0.2, 50)

            // 返回按钮 - 左边
            Button {
                width: Math.min(parent.width * 0.3, 120)
                height: parent.height
                text: "返回"
                background: Rectangle {
                    color: "#444444"
                    radius: height / 2
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: Math.max(14, parent.width * 0.035)
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.centerIn: parent
                }
                onClicked: root.backRequested()
            }

            // 发布按钮 - 右边
            Button {
                width: Math.min(parent.width * 0.3, 120)
                height: parent.height
                text: "发布"
                background: Rectangle {
                    color: "#FF2C5C"
                    radius: height / 2
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: Math.max(14, parent.width * 0.035)
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.centerIn: parent
                }
                onClicked: {
                    console.log("=== 发布信息 ===")
                    console.log("1. 视频描述:", descriptionArea.text)
                    console.log("2. 视频路径:", videoData.videoPath)
                    console.log("3. 原视频路径:", videoData.originalVideoPath)
                    console.log("4. 临时文件路径:", videoData.tempFilePath)
                    console.log("5. 是否合并过:", videoData.hasMerged)
                    console.log("6. 是否有音乐:", videoData.hasMusic)

                    // 清理临时文件（如果存在且需要清理）
                    cleanupTempFile()

                    console.log("发布流程完成（模拟）")
                    root.publishComplete()
                }
            }
        }
    }

    // 清理临时文件的函数
    function cleanupTempFile() {
        console.log("=== 清理临时文件检查 ===")

        // 确保有有效的临时文件路径
        if (!videoData.tempFilePath || videoData.tempFilePath === "") {
            console.log("✓ 没有临时文件需要清理")
            return
        }

        // 确保临时文件存在
        if (!fileutils || !fileutils.fileExists(videoData.tempFilePath)) {
            console.log("✓ 临时文件不存在，无需清理:", videoData.tempFilePath)
            return
        }

        // 安全保护：确保不是原视频
        if (videoData.originalVideoPath &&
            videoData.tempFilePath === videoData.originalVideoPath) {
            console.log("⚠️ 安全保护：临时文件路径与原视频相同，不删除")
            return
        }

        // 确保确实合并过
        if (!videoData.hasMerged) {
            console.log("⚠️ 未合并过，不删除临时文件")
            return
        }

        console.log("准备删除临时文件:", videoData.tempFilePath)

        // 删除临时文件
        if (fileutils.deleteFile(videoData.tempFilePath)) {
            console.log("✓ 临时文件删除成功")

            // 获取临时文件所在目录
            var tempDir = fileutils.getFileDirectory(videoData.tempFilePath)
            console.log("临时文件目录:", tempDir)

        } else {
            console.log("✗ 临时文件删除失败")
        }

        // 确认原视频安全
        if (videoData.originalVideoPath &&
            fileutils.fileExists(videoData.originalVideoPath)) {
            console.log("✓ 原视频安全:", fileutils.getFileName(videoData.originalVideoPath))
        }
    }
}
