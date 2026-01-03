/*神秘错误：
// 父ColumnLayout
// ├─ 第三行 (tabRow)
// │  ├─ 矩形1 (Layout.fillHeight: true)  ← 这个属性会向上传递！
// │  ├─ 矩形2 (Layout.fillHeight: true)
// │  └─ 矩形3 (Layout.fillHeight: true)
// └─ 第四行 (contentArea, Layout.fillHeight: true)

// 关键问题：当RowLayout的所有子元素都有 Layout.fillHeight: true 时，
RowLayout会认为自己应该填满可用高度！它忽略了外层的 Layout.preferredHeight: 40。
*/

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: _profileRectangle
    Layout.fillWidth: true
    Layout.fillHeight: true
    color: "#000000"

    property int currentProfileTab: 0
    // 个人主页 - 四行布局
    ColumnLayout {
        anchors.fill: parent
        spacing: 15

        // 第一行：用户头像和基本信息 - 占30%
        ColumnLayout {
            id: headerSection
            Layout.fillWidth: true
            Layout.preferredHeight: _profileRectangle.height * 0.3  // 总高度的30%
            spacing: 0
            Layout.alignment: Qt.AlignHCenter  // 水平居中
            Layout.topMargin: 20
            TapHandler {
                    onTapped: {
                        console.log("组件ID:", headerSection)
                        console.log("实际宽度:", headerSection.width)
                        console.log("实际高度:", headerSection.height)
                    }
            }
            // 头像
            Rectangle {
                Layout.preferredWidth: 150
                Layout.preferredHeight: 150
                radius: 75
                color: "#FF0050"
                border.color: "#FFFFFF"
                border.width: 2
                Layout.alignment: Qt.AlignHCenter
            }

            // 用户信息
            // 用户名
            Text {
                text: "抖音用户"
                color: "#FFFFFF"
                font.pixelSize: 20
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }

            // 抖音号
            Text {
                text: "抖音号: douyin123456"
                color: "#AAAAAA"
                font.pixelSize: 15
                Layout.alignment: Qt.AlignHCenter
            }

            // 获赞、关注、粉丝
            RowLayout {
                spacing: 15
                Layout.alignment: Qt.AlignHCenter

                // 获赞
                ColumnLayout {
                    spacing: 2
                    Text {
                        text: "1.2w"
                        color: "#FFFFFF"
                        font.pixelSize: 20
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: "获赞"
                        color: "#AAAAAA"
                        font.pixelSize: 15
                        Layout.alignment: Qt.AlignHCenter
                    }
                }

                // 关注
                ColumnLayout {
                    spacing: 2
                    Text {
                        text: "128"
                        color: "#FFFFFF"
                        font.pixelSize: 20
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: "关注"
                        color: "#AAAAAA"
                        font.pixelSize: 15
                        Layout.alignment: Qt.AlignHCenter
                    }
                }

                // 粉丝
                ColumnLayout {
                    spacing: 2
                    Text {
                        text: "2.5k"
                        color: "#FFFFFF"
                        font.pixelSize: 20
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: "粉丝"
                        color: "#AAAAAA"
                        font.pixelSize: 15
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }

        // 第二行：两个按钮 - 固定高度
        RowLayout {
            id: buttonRow
            Layout.fillWidth: true
            Layout.preferredHeight: 40  // 固定高度
            spacing: 10
            TapHandler {
                    onTapped: {
                        console.log("组件ID:", buttonRow)
                        console.log("实际宽度:", buttonRow.width)
                        console.log("实际高度:", buttonRow.height)
                    }
            }
            // 编辑个人信息按钮
            Rectangle {
                Layout.preferredWidth: 0  // 让两个按钮等宽
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                color: "#333333"
                radius: 5

                Text {
                    text: "编辑资料"
                    color: "#FFFFFF"
                    font.pixelSize: 16
                    anchors.centerIn: parent
                }

                TapHandler {
                    onTapped: console.log("编辑个人信息")
                }
            }

            // // 加好友按钮
            // Rectangle {
            //     Layout.preferredWidth: 0  // 让两个按钮等宽
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 40
            //     color: "#FF0050"
            //     radius: 5

            //     Text {
            //         text: "加好友"
            //         color: "#FFFFFF"
            //         font.pixelSize: 16
            //         anchors.centerIn: parent
            //     }

            //     TapHandler {
            //         onTapped: console.log("加好友")
            //     }
            // }
        }

        // 第三行：作品、推荐、喜欢按钮行 - 固定高度
        RowLayout {
            id: tabRow
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            spacing: 0

            // 当前选中的标签
            property int selectedIndex: 0  // 0:作品, 1:推荐, 2:喜欢

            // 作品按钮
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                color: "transparent"

                Text {
                    text: "作品"
                    color: tabRow.selectedIndex === 0 ? "#FFFFFF" : "#AAAAAA"
                    font.pixelSize: 16
                    font.bold: tabRow.selectedIndex === 0
                    anchors.centerIn: parent
                }

                // 底部红线
                Rectangle {
                    width: parent.width
                    height: 2
                    color: "#FF0050"
                    anchors.bottom: parent.bottom
                    visible: tabRow.selectedIndex === 0
                }

                TapHandler {
                    onTapped: {
                        console.log("切换到作品")
                        tabRow.selectedIndex = 0
                        currentProfileTab = 0
                    }
                }
            }

            // // 收藏按钮
            // Rectangle {
            //     Layout.fillWidth: true
            //     Layout.preferredHeight: 40
            //     color: "transparent"

            //     Text {
            //         text: "收藏"
            //         color: tabRow.selectedIndex === 1 ? "#FFFFFF" : "#AAAAAA"
            //         font.pixelSize: 16
            //         font.bold: tabRow.selectedIndex === 1
            //         anchors.centerIn: parent
            //     }

            //     // 底部红线
            //     Rectangle {
            //         width: parent.width
            //         height: 2
            //         color: "#FF0050"
            //         anchors.bottom: parent.bottom
            //         visible: tabRow.selectedIndex === 1
            //     }

            //     TapHandler {
            //         onTapped: {
            //             console.log("切换到推荐")
            //             tabRow.selectedIndex = 1
            //             currentProfileTab = 1
            //         }
            //     }
            // }

            // 喜欢按钮
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                color: "transparent"

                Text {
                    text: "喜欢"
                    color: tabRow.selectedIndex === 2 ? "#FFFFFF" : "#AAAAAA"
                    font.pixelSize: 16
                    font.bold: tabRow.selectedIndex === 2
                    anchors.centerIn: parent
                }

                // 底部红线
                Rectangle {
                    width: parent.width
                    height: 2
                    color: "#FF0050"
                    anchors.bottom: parent.bottom
                    visible: tabRow.selectedIndex === 2
                }

                TapHandler {
                    onTapped: {
                        console.log("切换到喜欢")
                        tabRow.selectedIndex = 2
                        currentProfileTab = 2
                    }
                }
            }
        }

        // 第四行：视频预览区域 - 占据剩余空间
        Loader {
            id: profileLoader
            Layout.fillWidth: true
            Layout.fillHeight: true

            // 根据currentTab加载不同页面
            sourceComponent: {
                if (currentProfileTab === 0) {
                    return _profileWroks
                }
                if (currentProfileTab === 1) {
                    return _profileCollects
                }
                if (currentProfileTab === 2) {
                    return _profileLikes
                }
            }
        }
        // 作品页面
        Component {
            id: _profileWroks
            ProfileWroks {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        // 喜欢页面
        Component {
            id: _profileLikes
            ProfileLikes {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
