import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#000000"

    signal publishComplete
    signal backRequested

    // 屏幕宽度，用于自适应
    property int screenWidth: parent ? parent.width : 375

    Column {
        anchors.fill: parent
        anchors.margins: screenWidth * 0.05
        spacing: screenWidth * 0.05

        // 添加文本 - 修复 TextArea 占位符颜色问题
        TextField {
            id: descriptionArea
            width: parent.width
            height: parent.height * 0.6  // 60%高度给描述
            placeholderText: "添加描述..."
            placeholderTextColor: "#888888"
            color: "white"
            font.pixelSize: screenWidth * 0.04
            // wrapMode: TextArea.Wrap
            // selectByMouse: true
            background: Rectangle {
                color: "#222222"
                radius: screenWidth * 0.02
            }
            padding: screenWidth * 0.03  // 内边距，避免文字贴着边
            leftPadding: screenWidth * 0.03
            // TextArea 没有 placeholderTextColor，使用默认颜色（灰色）
        }

        // 添加标签
        TextField {
            id: tagField
            width: parent.width
            height: screenWidth * 0.12
            placeholderText: "添加标签..."
            placeholderTextColor: "#888888"  // TextField 支持这个属性
            color: "white"
            font.pixelSize: screenWidth * 0.035
            background: Rectangle {
                color: "#222222"
                radius: screenWidth * 0.02
            }
            padding: screenWidth * 0.03
            leftPadding: screenWidth * 0.03
        }

        // 底部按钮区域
        Row {
            width: parent.width
            height: screenWidth * 0.12

            // 返回按钮 - 左边
            Button {
                width: screenWidth * 0.3
                height: parent.height
                text: "返回"
                background: Rectangle {
                    color: "#444444"
                    radius: height / 2
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: screenWidth * 0.035
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.centerIn: parent
                }
                onClicked: root.backRequested()
            }

            // 中间填充
            Item {
                width: parent.width - (screenWidth * 0.3 * 2)  // 减去两个按钮宽度
                height: parent.height
            }

            // 发布按钮 - 右边
            Button {
                width: screenWidth * 0.3
                height: parent.height
                text: "发布"
                background: Rectangle {
                    color: "#FF2C5C"
                    radius: height / 2
                }
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.pixelSize: screenWidth * 0.035
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    anchors.centerIn: parent
                }
                onClicked: {
                    console.log("发布视频")
                    console.log("描述:", descriptionArea.text)
                    console.log("标签:", tagField.text)
                    root.publishComplete()
                }
            }
        }
    }
}
