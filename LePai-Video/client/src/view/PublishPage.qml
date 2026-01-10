import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#000000"

    signal publishComplete
    signal backRequested

    Column {
        anchors.fill: parent
        anchors.margins: Math.min(parent.width * 0.05, 20)
        spacing: Math.min(parent.width * 0.05, 10)

        // 添加文本 - 修复 TextArea 占位符颜色问题
        TextField {
            id: descriptionArea
            width: parent.width
            height: parent.height * 0.6  // 60%高度给描述
            placeholderText: "添加描述..."
            placeholderTextColor: "#888888"
            color: "white"
            font.pixelSize: Math.max(14, parent.width * 0.04)
            background: Rectangle {
                color: "#222222"
                radius: Math.min(parent.width * 0.02, 8)
            }
            padding: Math.min(parent.width * 0.03, 10)
            leftPadding: Math.min(parent.width * 0.03, 10)
        }

        // 底部按钮区域
        Row {
            width: parent.width
            height: Math.min(parent.width * 0.12, 50)
            spacing:Math.min(parent.width * 0.2, 50)

            // 返回按钮 - 左边
            Button {
                width: Math.min(parent.width * 0.3, 120)
                height: parent.height
                text: "返回"
                background: Rectangle {
                    color: "#444444"
                    radius: height / 2
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: Math.max(14, parent.width * 0.035)
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.centerIn: parent
                }
                onClicked: root.backRequested()
            }
            // 发布按钮 - 右边
            Button {
                width: Math.min(parent.width * 0.3, 120)
                height: parent.height
                text: "发布"
                background: Rectangle {
                    color: "#FF2C5C"
                    radius: height / 2
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: Math.max(14, parent.width * 0.035)
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.centerIn: parent
                }
                onClicked: {
                    console.log("发布视频")
                    console.log("描述:", descriptionArea.text)
                    root.publishComplete()
                }
            }
        }
    }
}
