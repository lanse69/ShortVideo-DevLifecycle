import QtQuick
import QtQuick.Controls
import QtMultimedia

Rectangle {
    color: "#000000"
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
    TapHandler {
        onTapped: {
            if(mediaPlayer.playing){
                mediaPlayer.pause()
            }else{
                mediaPlayer.play()
            }
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
        }
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
