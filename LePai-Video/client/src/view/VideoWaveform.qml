import QtQuick
import QtQuick.Controls
import QtMultimedia

Rectangle {
    id: videoWaveformRoot
    // height: 80
    color: "transparent"

    // 属性
    property var mediaPlayer: null
    property string filePath: ""
    property string type: "video"
    property real duration: 0
    property real position: 0

    // 当属性变化时更新显示
    onFilePathChanged: updateDisplay()
    onDurationChanged: updateDisplay()
    onPositionChanged: updateDisplay()

    // 标题和文件名
    Label {
        id: titleLabel
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 5
        text: "视频预览条"
        color: "#4a90e2"
        font.pixelSize: 12
        font.bold: true
    }

    Label {
        id: fileNameLabel
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 5
        text: filePath ? filePath.split('/').pop() : "未选择文件"
        color: "#888"
        font.pixelSize: 10
        elide: Text.ElideMiddle
        width: parent.width - 100
    }

    // 时间显示
    Label {
        id: timeLabel
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 5
        text: formatTime(position) + " / " + formatTime(duration)
        color: "white"
        font.pixelSize: 10
        visible: duration > 0
    }

    // 预览区域
    Rectangle {
        id: previewArea
        anchors.fill: parent
        anchors.margins: 25
        anchors.topMargin: 20
        color: "#2d2d2d"
        radius: 5

        // 进度条背景
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

        // 进度填充
        Rectangle {
            id: progressFill
            width: duration > 0 ? (position / duration) * parent.width : 0
            height: parent.height
            color: "#4a90e250"
            radius: 5
        }

        // 当前时间指示器
        Rectangle {
            id: timeIndicator
            width: 2
            height: parent.height
            color: "white"
            x: duration > 0 ? (position / duration) * parent.width - 1 : 0
            visible: duration > 0
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

    // 更新显示
    function updateDisplay() {
        // 这里可以添加更新逻辑
    }

    // 工具函数：格式化时间（用于时间标签）
    function formatTime(milliseconds) {
        if (!milliseconds || milliseconds <= 0) return "00:00"

        var seconds = Math.floor(milliseconds / 1000)
        var minutes = Math.floor(seconds / 60)
        var hours = Math.floor(minutes / 60)

        seconds = seconds % 60
        minutes = minutes % 60

        if (hours > 0) {
            return hours.toString().padStart(2, '0') + ":" +
                   minutes.toString().padStart(2, '0') + ":" +
                   seconds.toString().padStart(2, '0')
        } else {
            return minutes.toString().padStart(2, '0') + ":" +
                   seconds.toString().padStart(2, '0')
        }
    }

    // 工具函数：格式化刻度时间（简化显示）
    function formatTimeForScale(milliseconds) {
        if (!milliseconds || milliseconds <= 0) return "0:00"

        var seconds = Math.floor(milliseconds / 1000)
        var minutes = Math.floor(seconds / 60)

        seconds = seconds % 60

        // 简化显示格式
        return minutes + ":" + seconds.toString().padStart(2, '0')
    }
}
