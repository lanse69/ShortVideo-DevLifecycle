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
    property bool showLoginPage:true// 控制登录页面显示
    property bool waslogin:authManager.wasLogin

    //登出按钮
    Button {
       id: logoutButton
       anchors.right: parent.right
       anchors.top: parent.top
       anchors.margins: 15
       width: 60
       height: 32
       text: "登出"
       visible: waslogin  // 只在登录时显示
       z: 5

       background: Rectangle {
           color: logoutButton.down ? "#c0392b" : (logoutButton.hovered ? "#e74c3c" : "#e74c3c")
           radius: 6
           border.color: "#fff"
           border.width: 1
       }

       contentItem: Text {
           text: logoutButton.text
           color: "#FFFFFF"
           font.pixelSize: 14
           font.bold: true
           horizontalAlignment: Text.AlignHCenter
           verticalAlignment: Text.AlignVCenter
       }

       onClicked: {
           console.log("点击了登出按钮")
           authManager.logout()
       }
   }

    // 登出失败提示框
    Rectangle {
        id: logoutErrorTip
        anchors.centerIn: parent
        width: 200
        height: 50
        radius: 8
        color: "#333333"
        opacity: 0
        visible: opacity > 0
        z: 200  // 确保在最上层

        Text {
            anchors.centerIn: parent
            text: "登出失败"
            color: "#FFFFFF"
            font.pixelSize: 14
        }
    }
    // 监听登出信号
    Connections {
       target: authManager
       // 登出成功信号
       function onLogoutSuccess() {
           console.log("[ProfileTab] 登出成功")
           // 登出后显示登录页面
           showLoginPage = true
           authManager.loginMassage = ""
       }

       // 登出失败信号
       function onLogoutFailed(errorMessage) {
           console.log("[ProfileTab] 登出失败:", errorMessage)
           showLogoutError()
       }
    }

    // 显示登出错误提示的方法
    function showLogoutError() {
        // 显示提示框
        logoutErrorTip.opacity = 1

        // 3秒后淡出
        logoutErrorTimer.restart()
    }

    // 登出错误提示的定时器
    Timer {
        id: logoutErrorTimer
        interval: 3000  // 3秒
        onTriggered: {
            // 淡出动画
            logoutErrorTip.opacity = 0
        }
    }

    // 登录页面（半透明覆盖层）
     Rectangle {
         id: loginOverlay
         anchors.fill: parent
         color: "#80000000"  // 半透明黑色背景
         visible: showLoginPage&&!waslogin
         z: 100

         onVisibleChanged: {
            loginPage.usernametext=""
            loginPage.passwordtext=""
         }
         // 登录页面内容
         LoginPage {
             id: loginPage
             width: Math.min(parent.width * 0.85, 400)
             height: Math.min(parent.height * 0.7, 600)
             anchors.centerIn: parent
             radius: 15  // 圆角

             color: "white"
             onCloseRequested:{
             showLoginPage=false
            }
          }
     }

    // 个人主页
    ColumnLayout {
        anchors.fill: parent
        spacing: 15

        // 用户头像和基本信息
        ColumnLayout {
            id: headerSection
            Layout.fillWidth: true
            Layout.preferredHeight: _profileRectangle.height * 0.3
            spacing: 0
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 20
            TapHandler {
                    onTapped: {
                        console.log("组件ID:", headerSection)
                        console.log("实际宽度:", headerSection.width)
                        console.log("实际高度:", headerSection.height)
                    }
            }

            Rectangle {
                Layout.preferredWidth: 120
                Layout.preferredHeight: 120
                radius: 60
                border.color: "#FFFFFF"
                border.width: 2
                Layout.alignment: Qt.AlignHCenter
                clip: true

                Image {
                    anchors.fill: parent
                    anchors.margins: 2 
                    source: authManager.currentUser.avatarUrl
                    fillMode: Image.PreserveAspectCrop
                }
            }

            // 用户信息
            // 用户名
            Text {
                text: authManager.currentUser.username
                color: "#FFFFFF"
                font.pixelSize: 20
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }


            // 关注、粉丝
            RowLayout {
                spacing: 15
                Layout.alignment: Qt.AlignHCenter

                // 关注
                ColumnLayout {
                    spacing: 2
                    Text {
                        text: authManager.currentUser.followingCount
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
                        text: authManager.currentUser.followerCount
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

        RowLayout {
            id: buttonRow
            Layout.fillWidth: true
            Layout.preferredHeight: 40
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
                Layout.preferredWidth: 0
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

        RowLayout {
            id: tabRow
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            spacing: 0

            property int selectedIndex: 0  // 0:作品, 1:喜欢

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

        Loader {
            id: profileLoader
            Layout.fillWidth: true
            Layout.fillHeight: true

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
