// MergeControlPanel.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: controlPanel
    width: 400
    color: "#2d2d2d"
    clip: true

    // 属性
    property string videoSource: ""
    property string audioSource: ""
    property real videoDuration: 0
    property real audioDuration: 0
    property real videoPosition: 0
    property real audioPosition: 0

    // 信号
    signal mergeRequested(videoStart: real, videoEnd: real, audioStart: real, audioEnd: real)
    signal closed()

    // 内部属性
    property string videoStartText: "0.000"
    property string videoEndText: "10.000"
    property string audioStartText: "0.000"
    property string audioEndText: "10.000"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        // 标题栏
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "🎬 合并设置"
                color: "white"
                font.bold: true
                font.pixelSize: 18
            }

            Item { Layout.fillWidth: true }

            Button {
                text: "×"
                background: Rectangle {
                    color: "transparent"
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 18
                }
                onClicked: controlPanel.closed()
            }
        }

        // 视频时间设置
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: "📹 视频时间范围"
                color: "#4a90e2"
                font.bold: true
                font.pixelSize: 14
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                TextField {
                    id: videoStartInput
                    placeholderText: "开始(秒)"
                    Layout.fillWidth: true
                    validator: DoubleValidator { bottom: 0; decimals: 3 }
                    background: Rectangle {
                        color: "#1a1a1a"
                        radius: 4
                        border.color: videoStartInput.activeFocus ? "#4a90e2" : "#555"
                    }
                    color: "white"
                    font.pixelSize: 12
                    selectByMouse: true
                    text: videoStartText
                    onTextChanged: videoStartText = text
                }

                Label {
                    text: "→"
                    color: "#888"
                }

                TextField {
                    id: videoEndInput
                    placeholderText: "结束(秒)"
                    Layout.fillWidth: true
                    validator: DoubleValidator { bottom: 0; decimals: 3 }
                    background: Rectangle {
                        color: "#1a1a1a"
                        radius: 4
                        border.color: videoEndInput.activeFocus ? "#4a90e2" : "#555"
                    }
                    color: "white"
                    font.pixelSize: 12
                    selectByMouse: true
                    text: videoEndText
                    onTextChanged: videoEndText = text
                }

                Button {
                    text: "当前"
                    Layout.preferredWidth: 80
                    onClicked: {
                        videoStartInput.text = (videoPosition / 1000).toFixed(3)
                    }
                }
            }
        }

        // 音频时间设置
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            Label {
                text: "🎵 音频时间范围"
                color: "#50c878"
                font.bold: true
                font.pixelSize: 14
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                TextField {
                    id: audioStartInput
                    placeholderText: "开始(秒)"
                    Layout.fillWidth: true
                    validator: DoubleValidator { bottom: 0; decimals: 3 }
                    background: Rectangle {
                        color: "#1a1a1a"
                        radius: 4
                        border.color: audioStartInput.activeFocus ? "#50c878" : "#555"
                    }
                    color: "white"
                    font.pixelSize: 12
                    selectByMouse: true
                    text: audioStartText
                    onTextChanged: audioStartText = text
                }

                Label {
                    text: "→"
                    color: "#888"
                }

                TextField {
                    id: audioEndInput
                    placeholderText: "结束(秒)"
                    Layout.fillWidth: true
                    validator: DoubleValidator { bottom: 0; decimals: 3 }
                    background: Rectangle {
                        color: "#1a1a1a"
                        radius: 4
                        border.color: audioEndInput.activeFocus ? "#50c878" : "#555"
                    }
                    color: "white"
                    font.pixelSize: 12
                    selectByMouse: true
                    text: audioEndText
                    onTextChanged: audioEndText = text
                }

                Button {
                    text: "当前"
                    Layout.preferredWidth: 80
                    onClicked: {
                        audioStartInput.text = (audioPosition / 1000).toFixed(3)
                    }
                }
            }
        }

        // 时长信息
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: "#1a1a1a"
            radius: 5

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: "📹 视频时长:"
                        color: "#4a90e2"
                        font.pixelSize: 12
                    }

                    Label {
                        id: videoDurationLabel
                        text: formatTime(videoDuration)
                        color: "white"
                        font.pixelSize: 12
                        Layout.fillWidth: true
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: "🎵 音频时长:"
                        color: "#50c878"
                        font.pixelSize: 12
                    }

                    Label {
                        id: audioDurationLabel
                        text: formatTime(audioDuration)
                        color: "white"
                        font.pixelSize: 12
                        Layout.fillWidth: true
                    }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Label {
                        text: "时长匹配:"
                        color: "#aaa"
                        font.pixelSize: 12
                    }

                    Label {
                        id: durationMatchLabel
                        text: calculateMatchStatus()
                        color: durationMatchLabel.text.includes("✅") ? "#50c878" : "#ff6b6b"
                        font.pixelSize: 12
                        font.bold: true
                        Layout.fillWidth: true
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }

        // 合并按钮
        Button {
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            text: "🚀 开始合并"
            enabled: videoSource && audioSource
            background: Rectangle {
                color: parent.enabled ? "#ff6b6b" : "#666"
                radius: 5
            }
            contentItem: Text {
                text: parent.text
                color: "white"
                font.bold: true
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            onClicked: {
                var videoStart = parseFloat(videoStartInput.text) * 1000
                var videoEnd = parseFloat(videoEndInput.text) * 1000
                var audioStart = parseFloat(audioStartInput.text) * 1000
                var audioEnd = parseFloat(audioEndInput.text) * 1000

                controlPanel.mergeRequested(videoStart, videoEnd, audioStart, audioEnd)
            }
        }
    }

    // 工具函数
    function formatTime(milliseconds) {
        if (!milliseconds || milliseconds <= 0) return "0.000s"

        var seconds = milliseconds / 1000
        return seconds.toFixed(3) + "s"
    }

    function calculateMatchStatus() {
        var videoStart = parseFloat(videoStartText) || 0
        var videoEnd = parseFloat(videoEndText) || 0
        var audioStart = parseFloat(audioStartText) || 0
        var audioEnd = parseFloat(audioEndText) || 0

        var videoDuration = Math.max(0, videoEnd - videoStart)
        var audioDuration = Math.max(0, audioEnd - audioStart)

        if (videoDuration <= 0 || audioDuration <= 0) return "❌"

        // 允许0.1秒差异
        var isMatched = Math.abs(videoDuration - audioDuration) < 0.1
        return isMatched ? "✅" : "❌"
    }

    // 更新显示
    function updateDisplay() {
        videoDurationLabel.text = formatTime(videoDuration)
        audioDurationLabel.text = formatTime(audioDuration)
        durationMatchLabel.text = calculateMatchStatus()
        durationMatchLabel.color = durationMatchLabel.text.includes("✅") ? "#50c878" : "#ff6b6b"
    }

    // 当属性变化时更新显示
    onVideoDurationChanged: updateDisplay()
    onAudioDurationChanged: updateDisplay()
    onVideoStartTextChanged: updateDisplay()
    onVideoEndTextChanged: updateDisplay()
    onAudioStartTextChanged: updateDisplay()
    onAudioEndTextChanged: updateDisplay()
}
