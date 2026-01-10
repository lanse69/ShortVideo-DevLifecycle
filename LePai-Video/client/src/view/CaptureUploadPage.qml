import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Rectangle {
    id: root
    color: "#000000"

    signal videoSelected(string filePath)

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 第一行：视频录制预览区域
        Rectangle {
            id: previewArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#222222"

            Text {
                anchors.centerIn: parent
                text: "录制视频预览区域"
                color: "white"
                font.pixelSize: parent.width * 0.05
            }
        }

        // 第二行：控制按钮区域
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(parent.height * 0.15, 150)
            color: "#000000"

            Item {
                anchors.centerIn: parent
                width: Math.min(parent.width * 0.8, 300)
                height: parent.height  // 容器高度跟随父容器

                // 左边：上传按钮
                Button {
                    id: uploadBtn
                    width: Math.min(parent.width * 0.25, 60)
                    height: width  // 关键：保持正方形，width = height
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    background: Rectangle {
                        color: "#666666"
                        radius: uploadBtn.width / 2  // 半径 = 宽度/2，确保圆形
                    }
                    contentItem: Text {
                        text: "上传"
                        color: "white"
                        font.pixelSize: Math.max(11, uploadBtn.width * 0.3)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: fileDialog.open()
                }

                // 中间：拍摄按钮
                Rectangle {
                    id: recordBtn
                    width: Math.min(parent.width * 0.30, 90)
                    height: width  // 关键：保持正方形
                    anchors.centerIn: parent
                    radius: width / 2  // 圆形边框
                    color: "transparent"
                    border.color: "#FF2C5C"
                    border.width: 3

                    Rectangle {
                        id: innerCircle
                        anchors.centerIn: parent
                        width: recordBtn.width * 0.6  // 内圆直径 = 外圆的60%
                        height: width  // 保持正方形
                        radius: width / 2  // 圆形内圆
                        color: "#FF2C5C"
                    }
                }

                // 右边：空白矩形
                Rectangle {
                    id: placeholder
                    width: Math.min(parent.width * 0.25, 60)
                    height: width  // 保持正方形
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    color: "#000000"
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "选择视频文件"
        nameFilters: ["视频文件 (*.mp4 *.mov *.avi)", "所有文件 (*)"]
        onAccepted: videoSelected(currentFile)
    }
}
