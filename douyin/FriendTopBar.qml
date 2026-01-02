// FriendsTopBar.qml - 好友页面顶部导航栏
import QtQuick
import QtQuick.Layouts

Rectangle {
    height: 60
    color: "#000000"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 左侧：好友标题
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 60

            Text {
                text: "👥"
                color: "#FFFFFF"
                font.pixelSize: 22
                font.bold: true
                anchors {
                    left: parent.left
                    leftMargin: 15
                    verticalCenter: parent.verticalCenter
                }
            }
        }

        // 右侧：相机按钮（发日常）
        Rectangle {
            Layout.preferredWidth: 80
            Layout.preferredHeight: 60
            color: "transparent"
            ColumnLayout {
                    anchors.centerIn: parent
                    spacing: 0
                // 相机图标
                Text {
                    text: "📷"
                    color: "#FFFFFF"
                    font.pixelSize: 22
                    Layout.alignment: Qt.AlignHCenter
                    // 点击动画
                    TapHandler {
                        onTapped: {
                            console.log("打开相机发日常")
                            cameraClickAnimation.start()
                        }
                    }

                    // 点击动画效果
                    SequentialAnimation on scale {
                        id: cameraClickAnimation
                        running: false
                        NumberAnimation { to: 1.3; duration: 100 }
                        NumberAnimation { to: 1.0; duration: 100 }
                    }
                }

                // 相机文字
                Text {
                    text: "发日常"
                    color: "#FFFFFF"
                    font.pixelSize: 10
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }
}
