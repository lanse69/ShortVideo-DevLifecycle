import QtQuick
import QtQuick.Controls
import QtMultimedia
import QtQuick.Layouts

Rectangle {
    id: audioWaveformRoot
    color: "transparent"

    property var mediaPlayer: null
    property string filePath: ""
    property real duration: 0
    property real position: 0

    // 音频波形数据
    property var waveformData: []

    // 当属性变化时更新显示
    onFilePathChanged: {
        if (filePath) {
            generateWaveformData()
            canvas.requestPaint()
        }
    }
    onDurationChanged: {
        generateWaveformData()
        canvas.requestPaint()
    }
    onPositionChanged: canvas.requestPaint()

    // 控制条
    Rectangle {
        id: controlBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 30
        color: "transparent"

        RowLayout {
            anchors.fill: parent
            spacing: 10

            // 播放/暂停按钮
            Button {
                id: audioPlayBtn
                text: mediaPlayer && mediaPlayer.playbackState === MediaPlayer.PlayingState ? "⏸️" : "▶️"
                onClicked: {
                    if (mediaPlayer) {
                        if (mediaPlayer.playbackState === MediaPlayer.PlayingState)
                            mediaPlayer.pause()
                        else
                            mediaPlayer.play()
                    }
                }
                enabled: filePath && mediaPlayer
                background: Rectangle {
                    color: "#50c878"
                    radius: 3
                }
                contentItem: Text {
                    text: audioPlayBtn.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                Layout.preferredWidth: 40
                Layout.preferredHeight: 25
            }

            // 占位空间
            Item {
                Layout.fillWidth: true
            }

            // 时间显示区域 - 放在最右边
            Label {
                id: timeLabel
                text: formatTime(position) + " / " + formatTime(duration)
                color: "white"
                font.pixelSize: 12
                Layout.alignment: Qt.AlignVCenter
                visible: duration > 0
                horizontalAlignment: Text.AlignRight
            }
        }
    }

    // 波形区域
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: controlBar.bottom
        anchors.topMargin: 5
        height: 60
        color: "#2d2d2d"
        radius: 5

        // 背景和刻度
        Rectangle {
            id: background
            anchors.fill: parent
            color: "#1a1a1a"
            radius: 5

            // 刻度系统：每10%一个长刻度，每5%一个短刻度
            Repeater {
                model: 21  // 0-100%，每5%一个刻度
                Rectangle {
                    width: 1
                    height: index % 2 === 0 ? 15 : 8  // 长刻度15px，短刻度8px
                    color: index % 2 === 0 ? "#666" : "#444"
                    x: index * (parent.width / 20)
                    y: parent.height - height
                }
            }

            // 长刻度标签（所有长刻度都显示时间）
            Repeater {
                model: 11  // 显示所有长刻度的时间（0%、10%、20%、...、100%）
                Text {
                    text: formatTimeForScale(index * (duration / 10))
                    color: "#999"
                    font.pixelSize: 8
                    x: index * (parent.width / 10) - width/2
                    y: parent.height - 30
                }
            }
        }

        // 音频波形画布
        Canvas {
            id: canvas
            anchors.fill: parent

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)

                if (waveformData.length > 0 && duration > 0) {
                    var progress = position / duration
                    var centerY = height / 2
                    var maxAmplitude = centerY * 0.7
                    var progressX = progress * width
                    var playedSamples = Math.floor(progress * waveformData.length)

                    // 绘制完整波形（灰色）
                    ctx.strokeStyle = "#555"
                    ctx.lineWidth = 1
                    ctx.beginPath()

                    for (var i = 0; i < waveformData.length; i++) {
                        var x = i * (width / waveformData.length)
                        var amplitude = waveformData[i] * maxAmplitude
                        var y = centerY - amplitude

                        if (i === 0) {
                            ctx.moveTo(x, y)
                        } else {
                            ctx.lineTo(x, y)
                        }
                    }
                    ctx.stroke()

                    // 绘制已播放部分的波形（绿色）
                    if (playedSamples > 0) {
                        ctx.strokeStyle = "#50c878"
                        ctx.lineWidth = 1.5
                        ctx.beginPath()

                        for (var j = 0; j < playedSamples; j++) {
                            var x2 = j * (width / waveformData.length)
                            var amplitude2 = waveformData[j] * maxAmplitude
                            var y2 = centerY - amplitude2

                            if (j === 0) {
                                ctx.moveTo(x2, y2)
                            } else {
                                ctx.lineTo(x2, y2)
                            }
                        }
                        ctx.stroke()

                        // 绘制波形填充（半透明绿色）
                        ctx.fillStyle = "#50c87820"
                        ctx.beginPath()
                        ctx.moveTo(0, centerY)

                        for (var k = 0; k < playedSamples; k++) {
                            var x3 = k * (width / waveformData.length)
                            var amplitude3 = waveformData[k] * maxAmplitude
                            var y3 = centerY - amplitude3
                            ctx.lineTo(x3, y3)
                        }

                        ctx.lineTo(playedSamples * (width / waveformData.length), centerY)
                        ctx.closePath()
                        ctx.fill()
                    }

                    // 绘制当前时间指示器
                    if (duration > 0) {
                        ctx.strokeStyle = "white"
                        ctx.lineWidth = 2
                        ctx.beginPath()
                        ctx.moveTo(progressX, 0)
                        ctx.lineTo(progressX, height)
                        ctx.stroke()

                        // 绘制时间线圆点
                        ctx.fillStyle = "white"
                        ctx.beginPath()
                        ctx.arc(progressX, centerY, 4, 0, Math.PI * 2)
                        ctx.fill()

                        ctx.fillStyle = "#50c878"
                        ctx.beginPath()
                        ctx.arc(progressX, centerY, 2, 0, Math.PI * 2)
                        ctx.fill()
                    }
                }
            }
        }

        // 鼠标交互
        MouseArea {
            anchors.fill: parent
            onClicked: function(mouse) {
                if (duration > 0 && mediaPlayer) {
                    var newPosition = (mouse.x / width) * duration
                    mediaPlayer.position = newPosition
                }
            }
        }
    }

    // 生成音频波形数据
    function generateWaveformData() {
        waveformData = []
        var sampleCount = 400  // 波形采样点数

        for (var i = 0; i < sampleCount; i++) {
            var progress = i / sampleCount

            // 模拟真实音频波形
            var amplitude = 0

            if (progress < 0.2) {
                // 开头：渐入
                amplitude = Math.sin(progress * 50) * 0.3 * progress * 5
            } else if (progress < 0.5) {
                // 主歌部分
                amplitude = Math.sin(progress * 40) * 0.4 +
                           Math.sin(progress * 80 + 0.5) * 0.2
            } else if (progress < 0.8) {
                // 副歌部分
                amplitude = Math.sin(progress * 60) * 0.5 +
                           Math.sin(progress * 120) * 0.3
            } else {
                // 结尾：渐出
                amplitude = (Math.sin(progress * 50) * 0.3) *
                           (1 - (progress - 0.8) * 5)
            }

            // 添加随机变化
            amplitude += (Math.random() - 0.5) * 0.05

            // 限制振幅范围
            amplitude = Math.max(-0.9, Math.min(0.9, amplitude))

            waveformData.push(amplitude)
        }
    }

    // 工具函数：格式化时间
    function formatTime(milliseconds) {
        if (!milliseconds || milliseconds <= 0) return "00:00"

        var seconds = Math.floor(milliseconds / 1000)
        var minutes = Math.floor(seconds / 60)

        seconds = seconds % 60

        return minutes.toString().padStart(2, '0') + ":" +
               seconds.toString().padStart(2, '0')
    }

    // 工具函数：格式化刻度时间
    function formatTimeForScale(milliseconds) {
        if (!milliseconds || milliseconds <= 0) return "0:00"

        var seconds = Math.floor(milliseconds / 1000)
        var minutes = Math.floor(seconds / 60)

        seconds = seconds % 60

        return minutes + ":" + seconds.toString().padStart(2, '0')
    }

    // 定时刷新波形
    Timer {
        interval: 50
        running: mediaPlayer && mediaPlayer.playbackState === MediaPlayer.PlayingState
        repeat: true
        onTriggered: canvas.requestPaint()
    }
}
