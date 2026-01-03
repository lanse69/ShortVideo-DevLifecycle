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

        // 第一行：视频录制预览区域 - 占满上部
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#222222"

            Text {
                anchors.centerIn: parent
                text: "录制视频预览区域"
                color: "white"
                font.pixelSize: 20
            }
        }

        // 第二行：控制按钮区域
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            color: "#000000"

            // 三个矩形并列
            Row {
                anchors.centerIn: parent
                spacing: 80

                // 左边：上传按钮
                Button {
                    width: 80
                    height: 80
                    anchors.verticalCenter: parent.verticalCenter
                    background: Rectangle {
                        color: "#666666"
                        radius: 40
                    }
                    contentItem: Text {
                        text: "上传"
                        color: "white"
                        font.pixelSize: 20
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: fileDialog.open()
                }

                // 中间：拍摄按钮
                Rectangle {
                    width: 100
                    height: 100
                    anchors.verticalCenter: parent.verticalCenter
                    radius: 50
                    color: "transparent"
                    border.color: "#FF2C5C"
                    border.width: 4

                    Rectangle {
                        anchors.centerIn: parent
                        width: 70
                        height: 70
                        radius: 35
                        color: "#FF2C5C"
                    }
                }

                // 右边：空白矩形
                Rectangle {
                    width: 80
                    height: 80
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
