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
        anchors.margins: 20
        spacing: 20

        // 添加文本
        TextArea {
            width: parent.width
            height: parent.height - 130  // 减去底部按钮区域高度
            placeholderText: "添加描述..."
            color: "white"
            font.pixelSize: 16
            wrapMode: TextArea.Wrap
            background: Rectangle {
                color: "#222222"
                radius: 10
            }
        }

        // 添加标签
        TextField {
            width: parent.width
            height: 50
            placeholderText: "添加标签..."
            color: "white"
            font.pixelSize: 14
            background: Rectangle {
                color: "#222222"
                radius: 10
            }
        }

        // 底部按钮区域
        Row {
            width: parent.width
            height: 50

            // 返回按钮 - 左边
            Button {
                width: 100
                height: 50
                text: "返回"
                background: Rectangle {
                    color: "#444444"
                    radius: 25
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: root.backRequested()
            }

            // 中间填充
            Item {
                width: parent.width - 200  // 总宽减去两个按钮宽度
                height: 50
            }

            // 发布按钮 - 右边
            Button {
                width: 100
                height: 50
                text: "发布"
                background: Rectangle {
                    color: "#FF2C5C"
                    radius: 25
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: 16
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: {
                    console.log("发布视频")
                    root.publishComplete()
                }
            }
        }
    }
}
