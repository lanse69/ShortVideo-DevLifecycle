// Player.qml - 最简单版本
import QtQuick
import QtMultimedia
import QtQuick.Controls

Rectangle {
    id: playerRoot
    property alias videoSource: player.source
    color: "black"

    MediaPlayer {
        id: player
        audioOutput: AudioOutput {}
        videoOutput: videoOutput
    }

    VideoOutput {
        id: videoOutput
        anchors.fill: parent
    }

    // 控制按钮
    Button {
        anchors.centerIn: parent
        width: 60
        height: 60
        visible: videoSource !== ""
        background: Rectangle {
            color: "#FFFFFF"
            radius: width / 2
            opacity: 0.7
        }
        contentItem: Text {
            text: player.playbackState === MediaPlayer.PlayingState ? "❚❚" : "▶"
            color: "#000000"
            font.pixelSize: 24
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        onClicked: {
            if (player.playbackState === MediaPlayer.PlayingState) {
                player.pause()
            } else {
                player.play()
            }
        }
    }

    Component.onCompleted: {
        if (videoSource !== "") {
            console.log("Player开始播放:", videoSource)
            player.play()
        }
    }
}
