import QtQuick
import QtQuick.Controls
import QtMultimedia

Item {
    // width: 800
    // height: 600
    visible: true
    property alias playerSource: mediaPlayer.source
    property alias mediaPlayer: mediaPlayer

    // 媒体播放器
    MediaPlayer {
        id: mediaPlayer
        loops: MediaPlayer.Infinite
        audioOutput: AudioOutput {
            volume: 0.7
        }
        videoOutput:videoOutput

        //autoPlay: true

        onErrorOccurred: {
            console.log("播放错误:", errorString)
        }

        onPlaybackStateChanged: {
            console.log("播放状态:", playbackState)
        }

        onMediaStatusChanged: {
            console.log("媒体状态:", mediaStatus)
        }
    }

    // 视频显示区域
    VideoOutput {
        id: videoOutput
        anchors.fill: parent

    }

    // 控制面板
    Rectangle {
        id: controlPanel
        anchors.bottom: parent.bottom
        width: parent.width
        height: 60
        color: "black"
        opacity: 0.8

        Row {
            anchors.centerIn: parent
            spacing: 20

            Button {
                text: "播放"
                onClicked: mediaPlayer.play()
            }

            Button {
                text: "暂停"
                onClicked: mediaPlayer.pause()
            }

            Button {
                text: "停止"
                onClicked: mediaPlayer.stop()
            }

            Slider {
                id: positionSlider
                width: 300
                from: 0
                to: mediaPlayer.duration
                value: mediaPlayer.position

                onMoved: {
                    mediaPlayer.position = value
                }
            }

            Text {
                text: formatTime(mediaPlayer.position) + " / " + formatTime(mediaPlayer.duration)
                color: "white"
                anchors.verticalCenter: parent.verticalCenter
            }

            Button {
                text: mediaPlayer.audioOutput.muted ? "🔇" : "🔊"
                onClicked: mediaPlayer.muted = !mediaPlayer.audioOutput.muted
            }
        }
    }

    // 时间格式化函数
    function formatTime(milliseconds) {
        var seconds = Math.floor(milliseconds / 1000)
        var minutes = Math.floor(seconds / 60)
        var hours = Math.floor(minutes / 60)

        seconds = seconds % 60
        minutes = minutes % 60

        return hours.toString().padStart(2, '0') + ":" +
               minutes.toString().padStart(2, '0') + ":" +
               seconds.toString().padStart(2, '0')
    }

    // 定时更新进度条
    Timer {
        interval: 100
        running: mediaPlayer.playbackState === MediaPlayer.PlayingState
        repeat: true
        onTriggered: {
            positionSlider.value = mediaPlayer.position
        }
    }
}
