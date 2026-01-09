import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia  // 添加这行

Rectangle {
    id: editVideo
    color: "#000000"

    property string videoSource: ""
    signal editComplete(var musicData)
    signal backRequested

    property bool musicSelectorVisible: false

    // // 在Component.onCompleted中加载音乐数据
    // Component.onCompleted: {
    //     if (musicViewModel) {
    //         musicViewModel.loadMusic()
    //     }
    //     // 连接音乐播放信号
    //     musicViewModel.playAudioRequested.connect(function(audioUrl) {
    //         console.log("接收到播放请求:", audioUrl)
    //         musicPlayer.source = audioUrl
    //         musicPlayer.play()
    //     })
    // }
    // 音乐播放器
    MediaPlayer {
        id: musicPlayer
        audioOutput: AudioOutput {
            volume: 0.5  // 设置音量（0.0到1.0）
        }
        onPlaybackStateChanged: {
            console.log("音乐播放状态:", playbackState === MediaPlayer.PlayingState ? "播放中" : "暂停")
        }
        onErrorOccurred: {
            console.error("音乐播放错误:", errorString)
        }
    }

    // 连接到ViewModel的播放信号
    Connections {
        target: musicViewModel
        enabled: musicViewModel != null
        function onPlayAudioRequested(audioUrl) {
            console.log("接收到播放请求:", audioUrl)
            if (musicPlayer) {
                musicPlayer.source = audioUrl
                musicPlayer.play()
            }
        }
    }

    // 在Component.onCompleted中加载音乐数据
    Component.onCompleted: {
        if (musicViewModel) {
            musicViewModel.loadMusic()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 视频预览区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#111111"

            // 如果没有视频，显示提示
            Text {
                anchors.centerIn: parent
                text: "视频预览区域"
                color: "white"
                font.pixelSize: parent.width * 0.05
                visible: videoSource === ""
            }

            // 使用Player组件
            Player {
                id: videoPlayer
                anchors.fill: parent
                visible: videoSource !== ""
                videoSource: editVideo.videoSource
            }

            // 显示已选音乐
            Text {
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottomMargin: 20
                text: musicViewModel && musicViewModel.isMusicSelected ?
                      "已选: " + musicViewModel.selectedMusicTitle + " - " + musicViewModel.selectedMusicArtist :
                      "未选择音乐"
                color: "#FF2C5C"
                font.pixelSize: 16
                visible: musicViewModel && musicViewModel.isMusicSelected
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

                // 选择音乐按钮
                Button {
                    Layout.preferredWidth: Math.min(parent.width * 0.4, 200)
                    Layout.preferredHeight: Math.min(parent.width * 0.12, 50)
                    background: Rectangle {
                        color: musicViewModel && musicViewModel.isMusicSelected ? "#444444" : "#666666"
                        radius: height / 2
                    }
                    contentItem: Text {
                        text: musicViewModel && musicViewModel.isMusicSelected ?
                              "已选音乐" : "选择音乐"
                        color: "white"
                        font.pixelSize: Math.max(15, parent.width * 0.04)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        anchors.centerIn: parent
                    }
                    onClicked: musicSelectorVisible = true
                }

                Item {
                    Layout.fillWidth: true
                }

                // 下一步按钮 - 总是可点击
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
                    onClicked: {
                        // 无论是否选择音乐都可以下一步
                        var musicData = null
                        if (musicViewModel && musicViewModel.isMusicSelected) {
                            musicData = musicViewModel.getCurrentMusicInfo()
                        }
                        editVideo.editComplete(musicData)  // 传入null表示不使用音乐
                    }
                    enabled: true  // 总是可用
                }

                Item {
                    Layout.fillWidth: true
                }
            }
        }
    }

    // 音乐选择弹出层
    Rectangle {
        id: musicSelector
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: parent.height * 0.7
        color: "#1A1A1A"
        visible: musicSelectorVisible
        z: 5

        y: musicSelectorVisible ? 0 : height

        Behavior on y {
            NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: musicSelectorVisible = false
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Math.min(parent.width * 0.05, 20)
            spacing: Math.min(parent.width * 0.03, 10)
            // 标题行 - 修改为：
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(parent.width * 0.1, 40)

                Text {
                    text: "选择音乐"
                    color: "white"
                    font.pixelSize: Math.max(16, parent.width * 0.045)
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                }

                // 播放/暂停按钮
                Button {
                    Layout.preferredWidth: Math.min(parent.width * 0.1, 30)
                    Layout.preferredHeight: Math.min(parent.width * 0.1, 30)
                    visible: musicViewModel && musicViewModel.isMusicSelected
                    background: Rectangle {
                        color: "transparent"
                    }
                    contentItem: Text {
                        text: musicPlayer.playbackState === MediaPlayer.PlayingState ? "❚❚" : "▶"
                        color: "#FF2C5C"
                        font.pixelSize: Math.max(16, parent.width * 0.045)
                        font.bold: true
                    }
                    onClicked: {
                        if (musicPlayer.playbackState === MediaPlayer.PlayingState) {
                            musicPlayer.pause()
                            if (musicViewModel) {
                                musicViewModel.pauseMusic()
                            }
                        } else {
                            if (musicViewModel && musicViewModel.isMusicSelected) {
                                musicViewModel.playCurrentMusic()
                            }
                        }
                    }
                }

                Button {
                    Layout.preferredWidth: Math.min(parent.width * 0.1, 30)
                    Layout.preferredHeight: Math.min(parent.width * 0.1, 30)
                    text: "×"
                    background: Rectangle {
                        color: "transparent"
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: Math.max(16, parent.width * 0.045)
                        font.bold: true
                    }
                    onClicked: musicSelectorVisible = false
                }
            }
            // 音乐列表
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ListView {
                    id: musicListView
                    model: musicViewModel ? musicViewModel.musicList : null
                    delegate: musicItemDelegate
                    spacing: Math.min(parent.width * 0.02, 8)

                    // 如果没有数据，显示提示
                    Label {
                        anchors.centerIn: parent
                        text: "没有找到音乐"
                        color: "#666666"
                        visible: !musicViewModel || musicListView.count === 0
                    }
                }
            }
        }
    }

    // 音乐项委托 - 修改为：
    Component {
        id: musicItemDelegate

        Button {
            width: musicListView.width
            height: Math.min(musicListView.width * 0.2, 80)
            property bool isCurrentSelected: musicViewModel && musicViewModel.selectedIndex === index
            property bool isCurrentPlaying: musicViewModel && musicViewModel.isPlaying &&
                                          musicViewModel.selectedIndex === index

            background: Rectangle {
                color: isCurrentSelected ? "#222222" : "#111111"
                radius: Math.min(width * 0.05, 8)
                border.color: isCurrentSelected ? "#FF2C5C" : "transparent"
                border.width: 2
            }
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.margins: Math.min(parent.width * 0.03, 10)

                // 播放状态图标
                Rectangle {
                    Layout.preferredWidth: Math.min(parent.width * 0.12, 40)
                    Layout.preferredHeight: Math.min(parent.width * 0.12, 40)
                    radius: Math.min(width * 0.2, 6)
                    color: "#333333"

                    Text {
                        anchors.centerIn: parent
                        text: isCurrentPlaying ? "▶" : "♪"
                        color: "white"
                        font.pixelSize: Math.max(14, parent.width * 0.3)
                    }
                }

                Column {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: Math.min(parent.width * 0.01, 4)

                    Text {
                        text: model.title || "未知标题"
                        color: isCurrentPlaying ? "#FF2C5C" : "white"
                        font.pixelSize: Math.max(14, parent.width * 0.035)
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Text {
                        text: (model.artist || "未知歌手") + " · " + (model.formattedDuration || "00:00")
                        color: isCurrentPlaying ? "#FF9955" : "#AAAAAA"
                        font.pixelSize: Math.max(12, parent.width * 0.03)
                    }
                }

                Rectangle {
                    Layout.preferredWidth: Math.min(parent.width * 0.07, 25)
                    Layout.preferredHeight: Math.min(parent.width * 0.07, 25)
                    radius: width / 2
                    color: isCurrentSelected ? "#FF2C5C" : "#555555"

                    Text {
                        anchors.centerIn: parent
                        text: isCurrentSelected ? "✓" : ""
                        color: "white"
                        font.pixelSize: Math.max(12, parent.width * 0.3)
                        font.bold: true
                    }
                }
            }

            onClicked: {
                if (musicViewModel) {
                    if (isCurrentSelected && musicViewModel.isPlaying) {
                        // 点击正在播放的音乐，暂停
                        musicViewModel.pauseMusic()
                    } else if (isCurrentSelected) {
                        // 点击已选中的音乐（但未播放），播放
                        musicViewModel.playCurrentMusic()
                    } else {
                        // 点击未选中的项，选择并播放
                        musicViewModel.selectMusic(index)
                        musicViewModel.playCurrentMusic()
                    }
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
}
