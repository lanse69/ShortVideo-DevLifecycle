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
import QtQuick.Controls

Rectangle {
    id: _profileRectangle
    Layout.fillWidth: true
    Layout.fillHeight: true
    color: "#000000"

    property int currentProfileTab: 0
    property bool showLoginPage: true  // 控制登录页面显示
    // 新增：登录页面（半透明覆盖层）
     Rectangle {
         id: loginOverlay
         anchors.fill: parent
         color: "#80000000"  // 半透明黑色背景
         visible: showLoginPage
         z: 100  // 确保在最上层

         // 登录页面内容 - 调整为更合适的手机比例
         LoginPage {
             id: loginPage
             width: Math.min(parent.width * 0.85, 400)  // 更宽一些，适合手机
             height: Math.min(parent.height * 0.7, 600)  // 更高一些
             anchors.centerIn: parent
             radius: 15  // 圆角

             // 使LoginPage背景为白色
             color: "white"

             onLoginSuccess: {
                 console.log("登录成功")
                 showLoginPage = false
             }

             onCloseRequested: {
                 console.log("关闭登录页面")
                 showLoginPage = false
             }
          }
     }

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
                Layout.preferredWidth: 120
                Layout.preferredHeight: 120
                radius: 60
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
