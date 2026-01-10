import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Rectangle {
    id: root
    color: "#000000"

    signal videoSelected(string filePath)

    // 屏幕宽度，用于自适应
    property int screenWidth: parent ? parent.width : 375
    property int screenHeight: parent ? parent.height : 667

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 第一行：视频录制预览区域 - 占满上部
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#222222"

            Text {
                anchors.centerIn: parent
                text: "录制视频预览区域"
                color: "white"
                font.pixelSize: screenWidth * 0.05  // 自适应字体大小
            }
        }

        // 第二行：控制按钮区域
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: screenHeight * 0.15  // 15%高度
            color: "#000000"

            // 三个矩形并列
            Row {
                anchors.centerIn: parent
                spacing: screenWidth * 0.1  // 10%宽度作为间距

                // 左边：上传按钮
                Button {
                    width: screenWidth * 0.15  // 15%宽度
                    height: screenWidth * 0.15  // 保持正方形
                    anchors.verticalCenter: parent.verticalCenter
                    background: Rectangle {
                        color: "#666666"
                        radius: width / 2  // 圆形
                    }
                    contentItem: Text {
                        text: "上传"
                        color: "white"
                        font.pixelSize: screenWidth * 0.04  // 自适应字体
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        anchors.centerIn: parent
                    }
                    onClicked: fileDialog.open()
                }

                // 中间：拍摄按钮
                Rectangle {
                    width: screenWidth * 0.18  // 18%宽度
                    height: screenWidth * 0.18  // 保持正方形
                    anchors.verticalCenter: parent.verticalCenter
                    radius: width / 2  // 圆形
                    color: "transparent"
                    border.color: "#FF2C5C"
                    border.width: 3

                    Rectangle {
                        anchors.centerIn: parent
                        width: parent.width * 0.7  // 70%大小
                        height: parent.width * 0.7
                        radius: width / 2  // 圆形
                        color: "#FF2C5C"
                    }
                }

                // 右边：空白矩形
                Rectangle {
                    width: screenWidth * 0.15  // 15%宽度
                    height: screenWidth * 0.15
                    anchors.verticalCenter: parent.verticalCenter
                    color: "#000000"
                }
            }
        }
    }

    FileDialog {
        id: fileDialog
        title: "选择视频文件"
        nameFilters: ["视频文件 (*.mp4 *.mov *.avi)", "所有文件 (*)"]
        onAccepted: {
            console.log("选择的文件:", currentFile)
            videoSelected(currentFile)
        }
    }
}
