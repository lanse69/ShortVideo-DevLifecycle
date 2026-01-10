import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import QtQuick.Dialogs

Rectangle {
    id: editVideo
    color: "#000000"

    property string videoSource: ""
    property string audioSource: ""
    property string selectedMusicTitle: ""
    property string selectedMusicArtist: ""
    property bool isMusicSelected: false


    // 新增：保存原始视频路径
    property string originalVideoSource: ""

    signal editComplete(var musicData)
    signal backRequested

    property bool musicSelectorVisible: false
    property bool mergeControlVisible: false

    // 添加新属性：跟踪合并状态
    property bool hasActuallyMerged: false
    property string mergedFilePath: ""

    // 在视频源设置时保存原始路径
       onVideoSourceChanged: {
           if (videoSource !== "" && originalVideoSource === "") {
               originalVideoSource = videoSource
               console.log("📌 保存原始视频路径:", originalVideoSource)
           }
       }

    // 文件选择对话框
    FileDialog {
        id: audioFileDialog
        title: "请选择音频文件"
        // currentFolder: StandardPaths.standardLocations(StandardPaths.MusicLocation)[0]
        nameFilters: ["音频文件 (*.mp3 *.wav *.aac *.m4a *.ogg *.flac)"]
        onAccepted: {
            var filePath = audioFileDialog.selectedFile.toString().replace("file://", "")
            console.log("选择的音频文件:", filePath)

            // 设置音频源
            audioSource = "file://" + filePath
            audioPlayer.source = audioSource
            audioPlayer.play()

            // 提取文件名作为标题
            var fileName = filePath.split('/').pop()
            var fileNameWithoutExt = fileName.split('.').slice(0, -1).join('.')
            selectedMusicTitle = fileNameWithoutExt
            selectedMusicArtist = "本地音乐"
            isMusicSelected = true

            console.log("音乐信息:", selectedMusicTitle, "-", selectedMusicArtist)
        }
    }

    // 视频播放器
    Player {
        id: videoPlayer
        source: editVideo.videoSource
        videoOutput: videoOutput

        onErrorChanged: {
            console.error("视频播放错误:", errorString)
        }

        Component.onCompleted: {
            if (editVideo.videoSource !== "") {
                console.log("视频播放器加载，源:", editVideo.videoSource)
                videoPlayer.play()
            }
        }
    }

    // 音频播放器（用于音频预览条和播放音乐）
    Player {
        id: audioPlayer
        audioOutput: AudioOutput {
            volume: 0.5
            muted: false
        }
        onPlaybackStateChanged: {
            console.log("音频播放状态:", playbackState === MediaPlayer.PlayingState ? "播放中" : "暂停")
        }
        onErrorOccurred: {
            console.error("音频播放错误:", errorString)
        }
    }

    // 主布局
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 视频预览区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#111111"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // 视频播放容器
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: "#333"

                    // VideoOutput显示视频画面
                    VideoOutput {
                        id: videoOutput
                        anchors.fill: parent
                        fillMode: VideoOutput.PreserveAspectFit

                        TapHandler {
                            onSingleTapped: {
                                if (videoPlayer.playbackState === MediaPlayer.PlayingState) {
                                    videoPlayer.pause()
                                } else {
                                    videoPlayer.play()
                                }
                            }
                        }

                        // 只在视频暂停时显示播放按钮
                        Button {
                            anchors.centerIn: parent
                            width: 60
                            height: 60
                            visible: videoSource !== "" && videoPlayer.playbackState !== MediaPlayer.PlayingState
                            background: Rectangle {
                                color: "#FFFFFF"
                                radius: width / 2
                                opacity: 0.7
                            }
                            contentItem: Text {
                                text: "▶"
                                color: "#000000"
                                font.pixelSize: 24
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                videoPlayer.play()
                            }
                        }
                    }

                    // 如果没有视频，显示提示
                    Text {
                        anchors.centerIn: parent
                        text: "视频预览区域"
                        color: "white"
                        font.pixelSize: 20
                        visible: videoSource === ""
                    }
                }
            }

            // 显示已选音乐
            Text {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 20
                text: isMusicSelected ?
                      "已选: " + selectedMusicTitle + " - " + selectedMusicArtist :
                      "未选择音乐"
                color: "#FF2C5C"
                font.pixelSize: 16
                visible: isMusicSelected
            }
            // 注意：已删除音频播放控制按钮
        }

        // 视频预览条区域
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 100
            color: "#1a1a1a"

            VideoWaveform {
                anchors.fill: parent
                anchors.margins: 10
                mediaPlayer: videoPlayer
                filePath: videoSource
                type: "video"
                position: videoPlayer.position
                duration: videoPlayer.duration
            }
        }

        // 音频预览条区域
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 140
            color: "#111111"

            AudioWaveform {
                anchors.fill: parent
                anchors.margins: 10
                mediaPlayer: audioPlayer
                filePath: audioSource
                position: audioPlayer.position
                duration: audioPlayer.duration
            }
        }

        // 底部控制区域
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: parent.height * 0.25
            color: "#000000"
            z: 10

            RowLayout {
                anchors.fill: parent
                spacing: parent.width * 0.05

                Item {
                    Layout.fillWidth: true
                }

                // 选择音乐按钮 - 改为打开文件对话框
                Button {
                    Layout.preferredWidth: Math.min(parent.width * 0.4, 200)
                    Layout.preferredHeight: Math.min(parent.width * 0.12, 50)
                    background: Rectangle {
                        color: isMusicSelected ? "#444444" : "#666666"
                        radius: height / 2
                    }
                    contentItem: Text {
                        text: isMusicSelected ? "已选音乐" : "选择音乐"
                        color: "white"
                        font.pixelSize: Math.max(15, parent.width * 0.04)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        anchors.centerIn: parent
                    }
                    onClicked: {
                        // 打开文件选择对话框
                        audioFileDialog.open()
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Button {
                    Layout.preferredWidth: Math.min(parent.width * 0.4, 200)
                    Layout.preferredHeight: Math.min(parent.width * 0.12, 50)
                    background: Rectangle {
                        color: "#FF2C5C"
                        radius: height / 2
                    }
                    contentItem: Text {
                        text: "下一步"
                        color: "white"
                        font.pixelSize: Math.max(15, parent.width * 0.04)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        anchors.centerIn: parent
                    }
                    // EditPage.qml - 只修改"下一步"按钮的onClicked
                    onClicked: {
                        var musicData = null

                        // 使用保存的原始路径
                        var originalVideoPath = originalVideoSource.replace("file://", "")
                        var currentVideoPath = videoSource.replace("file://", "")

                        console.log("=== 关键路径信息 ===")
                        console.log("原始视频源:", originalVideoSource)
                        console.log("当前视频源:", videoSource)
                        console.log("原始视频路径:", originalVideoPath)
                        console.log("当前视频路径:", currentVideoPath)
                        console.log("临时文件路径:", mergedFilePath)

                        // 构建完整视频数据
                        var fullVideoData = {
                            // 视频路径信息
                            videoPath: currentVideoPath,           // 当前播放的视频（可能合并后）
                            originalVideoPath: originalVideoPath,  // 原始视频路径（这里应该正确了）

                            // 临时文件信息
                            tempFilePath: mergedFilePath || "",

                            // 状态标记
                            hasMusic: isMusicSelected,
                            hasMerged: hasActuallyMerged,
                            hasTempFile: hasActuallyMerged && mergedFilePath !== "",

                            // 音乐信息（如果有）
                            musicInfo: null
                        }

                        // 如果有选择音乐
                        if (isMusicSelected) {
                            fullVideoData.musicInfo = {
                                title: selectedMusicTitle,
                                artist: selectedMusicArtist,
                                audioPath: audioSource.replace("file://", "")
                            }
                        }

                        console.log("编辑页面生成完整数据:", JSON.stringify(fullVideoData, null, 2))

                        // 传递给主容器
                        editVideo.editComplete(fullVideoData)
                    }
                    enabled: true
                }

                Item {
                    Layout.fillWidth: true
                }
            }
        }
    }

    // 顶部返回按钮
    Button {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: Math.min(parent.width * 0.05, 20)
        text: "← 返回"
        background: Rectangle {
            color: "transparent"
        }
        contentItem: Text {
            text: parent.text
            color: "white"
            font.pixelSize: Math.max(14, parent.width * 0.04)
        }
        onClicked: editVideo.backRequested()
    }

    // 合并控制按钮（悬浮在右上角）
    Button {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: Math.min(parent.width * 0.05, 20)
        text: "🎬 合并设置"
        background: Rectangle {
            color: "#4a90e2"
            radius: 5
        }
        contentItem: Text {
            text: parent.text
            color: "white"
            font.pixelSize: Math.max(14, parent.width * 0.04)
        }
        onClicked: mergeControlVisible = !mergeControlVisible
    }

    // 简单合并状态对话框
    Dialog {
        id: mergeStatusDialog
        modal: true
        closePolicy: Popup.NoAutoClose
        width: 300
        height: 150

        property string status: "preparing" // "preparing", "merging", "success", "failed"
        property string message: ""
        property string outputPath: ""

        background: Rectangle {
            color: "#2d2d2d"
            radius: 8
            border.color: "#444"
            border.width: 1
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            // 状态图标
            Rectangle {
                Layout.alignment: Qt.AlignCenter
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
                radius: 20
                color: {
                    if (mergeStatusDialog.status === "failed") return "#ff6b6b"
                    if (mergeStatusDialog.status === "success") return "#6bff6b"
                    return "#6b9bff"
                }

                Text {
                    anchors.centerIn: parent
                    text: {
                        if (mergeStatusDialog.status === "failed") return "✗"
                        if (mergeStatusDialog.status === "success") return "✓"
                        return "..."
                    }
                    color: "white"
                    font.pixelSize: 20
                    font.bold: true
                }
            }

            // 消息文本
            Label {
                Layout.fillWidth: true
                text: {
                    if (mergeStatusDialog.status === "preparing") return "准备合并..."
                    if (mergeStatusDialog.status === "merging") return "正在合并..."
                    if (mergeStatusDialog.status === "success") return "合并完成"
                    if (mergeStatusDialog.status === "failed") return mergeStatusDialog.message || "合并失败"
                    return ""
                }
                color: "white"
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
            }

            // 关闭按钮
            Button {
                Layout.alignment: Qt.AlignCenter
                Layout.topMargin: 10
                text: "关闭"
                visible: mergeStatusDialog.status === "success" || mergeStatusDialog.status === "failed"
                background: Rectangle {
                    color: parent.down ? "#555" : "#444"
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    mergeStatusDialog.close()
                }
            }
        }

        onOpened: {
            console.log("合并对话框打开，状态:", status)
        }

        onClosed: {
            console.log("合并对话框关闭")
            // 重置状态
            status = "preparing"
            message = ""
        }
    }

    // 使用独立的合并控制面板组件
    MergeControlPanel {
        id: mergeControlPanel
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 400
        visible: mergeControlVisible
        z: 20

        x: mergeControlVisible ? 0 : width

        Behavior on x {
            NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
        }

        videoSource: editVideo.videoSource
        audioSource: editVideo.audioSource
        videoDuration: videoPlayer.duration
        audioDuration: audioPlayer.duration
        videoPosition: videoPlayer.position
        audioPosition: audioPlayer.position

        // 在 MergeControlPanel 的 onMergeRequested 中添加详细调试
        onMergeRequested: function(videoStart, videoEnd, audioStart, audioEnd) {
            console.log("=== 开始合并调试 ===")
            console.log("1. videoaudiomerger 对象:", videoaudiomerger)
            console.log("2. 视频源:", videoSource)
            console.log("3. 音频源:", audioSource)
            console.log("4. 视频时间范围:", videoStart, "-", videoEnd, "ms")
            console.log("5. 音频时间范围:", audioStart, "-", audioEnd, "ms")

            // 第一步：显示准备对话框
            mergeStatusDialog.status = "preparing"
            mergeStatusDialog.message = "正在准备合并参数..."
            mergeStatusDialog.open()

            // 立即开始合并
            startVideoAudioMerge(videoStart, videoEnd, audioStart, audioEnd)
        }
        // 修改 startVideoAudioMerge 函数，使用 fileutils 生成路径
        function startVideoAudioMerge(videoStart, videoEnd, audioStart, audioEnd) {
            console.log("开始执行合并，参数:", videoStart, videoEnd, audioStart, audioEnd)

            if (videoaudiomerger) {
                try {
                    // 设置参数
                    videoaudiomerger.setVideoFile(videoSource)
                    videoaudiomerger.setVideoTimeRange(videoStart, videoEnd)
                    videoaudiomerger.setAudioFile(audioSource)
                    videoaudiomerger.setAudioTimeRange(audioStart, audioEnd)

                    // 使用 fileutils 生成合并文件路径
                    var originalVideoPath = videoSource.replace("file://", "")
                    var outputFile = fileutils.getMergedVideoPath(originalVideoPath)

                    if (!outputFile) {
                        throw new Error("无法生成输出文件路径")
                    }

                    console.log("输出文件:", outputFile)
                    videoaudiomerger.setOutputFile(outputFile)

                    // 保存合并文件路径
                    mergedFilePath = outputFile
                    hasActuallyMerged = true

                    // 直接显示合并中状态
                    mergeStatusDialog.status = "merging"
                    mergeStatusDialog.message =  "正在合并视频和音频..."
                    mergeStatusDialog.outputPath = outputFile

                    // 直接调用合并
                    videoaudiomerger.merge()

                    // 3秒后检查是否完成
                    checkMergeTimer.start()

                } catch (error) {
                    console.error("调用 videoaudiomerger 方法出错:", error)
                    mergeStatusDialog.status = "failed"
                    mergeStatusDialog.message = "合并出错: " + error
                }
            } else {
                console.error("videoaudiomerger 未定义!")
                mergeStatusDialog.status = "failed"
                mergeStatusDialog.message = "videoaudiomerger 未定义，请检查是否正确注册"
            }
        }

        // 修改合并完成后的逻辑
        Timer {
            id: checkMergeTimer
            interval: 3000 // 3秒后检查
            onTriggered: {
                // 检查合并是否真的完成了
                if (mergedFilePath && mergedFilePath !== "") {
                    // 1. 切换视频播放器到合并后的视频
                    console.log("切换到合并后的视频:", mergedFilePath)

                    // 停止当前视频播放
                    if (videoPlayer.playbackState !== MediaPlayer.StoppedState) {
                        videoPlayer.stop()
                    }

                    // 设置新的视频源
                    var newVideoSource = "file://" + mergedFilePath
                    videoPlayer.source = newVideoSource
                    editVideo.videoSource = newVideoSource  // 更新编辑页面的视频源

                    console.log("视频源已更新:", newVideoSource)

                    // 重新开始播放
                    videoPlayer.play()

                    // 2. 如果音乐正在播放，暂停音乐
                    if (audioPlayer.playbackState === MediaPlayer.PlayingState) {
                        audioPlayer.pause()
                        console.log("已暂停音乐播放")
                    }

                    // 3. 更新合并状态对话框
                    mergeStatusDialog.status = "success"
                    mergeStatusDialog.message = "合并完成，已切换到新视频"

                    console.log("合并完成，已切换到合并后的视频")
                } else {
                    // 没有合并文件路径，显示警告
                    console.warn("合并完成但没有生成文件路径")
                    mergeStatusDialog.status = "success"
                    mergeStatusDialog.message = "合并完成"
                }
            }
        }
        onClosed: {
            mergeControlVisible = false
        }
    }
}
