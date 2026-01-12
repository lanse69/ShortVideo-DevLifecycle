import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: loginPage
    color: "#ffffff"
    radius: 12

    property bool showRegisterPage: false
    signal closeRequested()

    Connections {
        target: authManager
        function onLoginSuccess() {
            console.log("[LoginPage] 登录成功，自动关闭窗口")
            // 延迟一会
            closeTimer.start()
        }
    }

    Timer {
        id: closeTimer
        interval: 500 // 0.5秒后关闭
        repeat: false
        onTriggered: {
            loginPage.closeRequested()
        }
    }

    // 右上角关闭按钮
    Button {
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 15
        width: 30
        height: 30
        text: "×"
        font.pixelSize: 24
        font.bold: true

        background: Rectangle {
            color: "transparent"
        }

        contentItem: Text {
            text: parent.text
            font: parent.font
            color: "#95a5a6"
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        onClicked: {
            console.log("点击了关闭按钮")
            closeRequested()
        }
    }

    // 注册界面
    RegisterPage {
        id: registerPage
        anchors.fill: parent
        visible: showRegisterPage
        z: parent.z + 10
        
        onClose: {
            showRegisterPage = false
            // 清空登录页的错误信息
            authManager.loginMassage = "" 
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: 25
        width: Math.min(parent.width * 0.85, 460)

        // 标题区域
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 12
             // 主标题
            Text {
                text: "乐拍视界"
                font.pixelSize: 32
                font.bold: true
                color: "#2c3e50"
                Layout.alignment: Qt.AlignHCenter
            }
        }

        // 登录表单卡片
        Rectangle {
            Layout.fillWidth: true
            Layout.topMargin: 15
            height: 350
            radius: 12
            color: "white"
            border.color: "#e0e6ed"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 30
                spacing: 20
                
                // 账号输入框
                ColumnLayout {
                    spacing: 6
                    Layout.fillWidth: true

                    Text {
                        text: "账 号"
                        font.pixelSize: 16
                        color: "#34495e"
                        font.bold: true
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 50
                        radius: 8
                        border.color: usernameInput.focus ? "#3498db" : "#dce1e8"
                        border.width: 2
                        color: usernameInput.focus ? "#f8fbff" : "#ffffff"

                        TextField {
                            id: usernameInput
                            anchors.fill: parent
                            placeholderText: "请输入账号"
                            font.pixelSize: 16
                            color: "#2c3e50"
                            placeholderTextColor: "#95a5a6"
                            background: Rectangle {
                                color: "transparent"
                            }
                            leftPadding: 12
                        }
                    }
                }

                // 密码输入框
                ColumnLayout {
                    spacing: 6
                    Layout.fillWidth: true

                    Text {
                        text: "密 码"
                        font.pixelSize: 16
                        color: "#34495e"
                        font.bold: true
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 50
                        radius: 8
                        border.color: passwordInput.focus ? "#3498db" : "#dce1e8"
                        border.width: 2
                        color: passwordInput.focus ? "#f8fbff" : "#ffffff"

                        // 眼睛按钮
                        Button {
                            anchors.right: parent.right
                            anchors.rightMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            width: 30
                            height: 30
                            text: passwordInput.echoMode === TextField.Password ? "👁️" : "👁️‍🗨️"
                            opacity: 0
                            Component.onCompleted: opacity = 1

                            background: Rectangle {
                                color: "transparent"
                            }

                            onClicked: {
                                if (passwordInput.echoMode === TextField.Password) {
                                    passwordInput.echoMode = TextField.Normal
                                } else {
                                    passwordInput.echoMode = TextField.Password
                                }
                            }
                        }

                        TextField {
                            id: passwordInput
                            anchors.fill: parent
                            anchors.rightMargin: 40
                            leftPadding: 12
                            rightPadding: 40
                            font.pixelSize: 16
                            color: "#2c3e50"
                            placeholderText: "请输入密码"
                            placeholderTextColor: "#95a5a6"
                            background: Rectangle {
                                color: "transparent"
                            }
                            echoMode: TextField.Password
                            verticalAlignment: TextInput.AlignVCenter
                        }
                    }
                }

                RowLayout {
                    spacing: 15
                    // 登录按钮
                    Button {
                        id: loginButton
                        Layout.fillWidth: true
                        Layout.topMargin: 10
                        height: 52
                        text: "登  录"
                        font.pixelSize: 20
                        font.bold: true
                        hoverEnabled: true

                        background: Rectangle {
                            radius: 10
                            color: loginButton.down ? "#2980b9" : (loginButton.hovered ? "#5dade2" : "#3498db")
                            opacity: loginButton.enabled ? 1 : 0.6
                        }

                        contentItem: Text {
                            text: loginButton.text
                            font: loginButton.font
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            console.log("点击登录按钮")
                            authManager.login(usernameInput.text, passwordInput.text)
                        }
                    }
                    // 注册按钮
                    Button {
                        id: registerButton
                        Layout.fillWidth: true
                        Layout.topMargin: 10
                        height: 52
                        text: "注  册"
                        font.pixelSize: 20
                        font.bold: true
                        hoverEnabled: true

                        background: Rectangle {
                            radius: 10
                            color: "white"
                            border.color: "#3498db"
                            border.width: 2
                        }

                        contentItem: Text {
                            text: registerButton.text
                            font: registerButton.font
                            color: "#3498db"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            console.log("点击注册按钮")
                            showRegisterPage = true
                        }
                    }
                }

                // 错误提示
                Text {
                    id: errorText
                    Layout.alignment: Qt.AlignHCenter
                    color: "#e74c3c"
                    visible: text !== ""
                    font.pixelSize: 14
                    font.bold: true
                    text: authManager.loginMassage
                }

                // 底部提示
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 5
                    text: "提示：请输入正确的账号和密码登录"
                    color: "#95a5a6"
                    font.pixelSize: 13
                }
            }
        }
    }
}