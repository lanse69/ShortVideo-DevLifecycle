import QtQuick
import QtQuick.Layouts

Rectangle {
    color: "transparent"

    signal pageChanged(int pageIndex)  // 0=推荐，1=关注

    // true=关注，false=推荐
    property bool isFollowTab: false

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // 左侧更多按钮
        Rectangle {
            Layout.preferredWidth: 80
            Layout.preferredHeight: 50
            color: "transparent"

            Text {
                text: "≡"
                color: "#FFFFFF"
                font.pixelSize: 18
                font.bold: true
                anchors.centerIn: parent
            }
        }

        // 中间推荐和关注切换
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 50

            RowLayout {
                anchors.centerIn: parent
                spacing: 40

                // 关注按钮
                Text {
                    text: "关注"
                    color: isFollowTab ? "#FFFFFF" : "#666666"
                    font.pixelSize: 18
                    font.bold: true

                    // 点击事件
                    TapHandler {
                        onTapped: {
                            isFollowTab = true
                            pageChanged(1)
                            console.log("切换到关注页面")
                        }
                    }
                }

                // 推荐按钮
                Text {
                    text: "推荐"
                    color: isFollowTab ? "#666666" : "#FFFFFF"
                    font.pixelSize: 18
                    font.bold: true

                    // 点击事件
                    TapHandler {
                        onTapped: {
                            isFollowTab = false
                            pageChanged(0)  // 发出信号，切换到推荐页
                            console.log("切换到推荐页面")
                        }
                    }
                }
            }
        }

        // 右侧搜索按钮
        Rectangle {
            Layout.preferredWidth: 80
            Layout.preferredHeight: 50
            color: "transparent"

            Text {
                text: "🔍"
                color: "#FFFFFF"
                font.pixelSize: 18
                anchors.centerIn: parent
            }
        }
    }
}
