import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LePaiClient

Rectangle {
    id: registerPageRoot
    color: "#ffffff"
    radius: 12

    signal close()

    Connections {
        target: authManager
        function onRegistrationSuccess() {
            console.log("[RegisterPage] 注册成功，准备返回登录页")
            // 延迟一会
            successTimer.start()
        }
    }

    Timer {
        id: successTimer
        interval: 1000 // 1秒后返回
        repeat: false
        onTriggered: {
            // 清空输入框
            usernameInput.text = ""
            passwordInput.text = ""
            // 发出关闭信号，返回登录页
            registerPageRoot.close()
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
        z: 10

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
            registerPageRoot.close()
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

            Text {
                text: "欢迎注册"
                font.pixelSize: 28
                font.bold: true
                color: "#2c3e50"
                Layout.alignment: Qt.AlignHCenter
            }
            Text {
                text: "加入乐拍视界，记录美好生活"
                font.pixelSize: 14
                color: "#7f8c8d"
                Layout.alignment: Qt.AlignHCenter
            }
        }

        // 表单区域
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 20

            // 账号输入框
            ColumnLayout {
                spacing: 6
                Layout.fillWidth: true

                Text {
                    text: "新账号"
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
                        placeholderText: "设置您的用户名"
                        font.pixelSize: 16
                        color: "#2c3e50"
                        placeholderTextColor: "#95a5a6"
                        background: Rectangle { color: "transparent" }
                        leftPadding: 12
                    }
                }
            }

            // 密码输入框
            ColumnLayout {
                spacing: 6
                Layout.fillWidth: true

                Text {
                    text: "设置密码"
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
                        text: passwordInput.echoMode === TextField.Password ? "显示" : "隐藏"
                        opacity: 0
                        Component.onCompleted: opacity = 1
                        background: Rectangle { color: "transparent" }
                        onClicked: {
                            passwordInput.echoMode = (passwordInput.echoMode === TextField.Password) ? TextField.Normal : TextField.Password
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
                        placeholderText: "设置您的登录密码"
                        placeholderTextColor: "#95a5a6"
                        echoMode: TextField.Password
                        background: Rectangle { color: "transparent" }
                        verticalAlignment: TextInput.AlignVCenter
                    }
                }
            }

            // 注册按钮
            Button {
                id: confirmRegisterBtn
                Layout.fillWidth: parent
                Layout.topMargin: 20
                height: 52
                text: "立即注册"
                font.pixelSize: 20
                font.bold: true
                hoverEnabled: true

                background: Rectangle {
                    radius: 10
                    color: confirmRegisterBtn.down ? "#27ae60" : (confirmRegisterBtn.hovered ? "#2ecc71" : "#27ae60")
                }

                contentItem: Text {
                    text: confirmRegisterBtn.text
                    font: confirmRegisterBtn.font
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    if(usernameInput.text === "" || passwordInput.text === "") {
                        statusLabel.text = "用户名或密码不能为空"
                        statusLabel.color = "#e74c3c"
                        return
                    }
                    statusLabel.text = "正在注册..."
                    statusLabel.color = "#3498db"
                    authManager.registerUser(usernameInput.text, passwordInput.text)
                }
            }

            // 状态/错误提示
            Text {
                id: statusLabel
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 10
                text: authManager.registMassage
                color: text.includes("成功") ? "#27ae60" : "#e74c3c"
                font.pixelSize: 14
                visible: text !== ""
            }

            // 返回登录
            Button {
                Layout.alignment: Qt.AlignHCenter
                text: "已有账号？返回登录"
                flat: true
                contentItem: Text {
                    text: parent.text
                    color: "#7f8c8d"
                    font.underline: true
                    horizontalAlignment: Text.AlignHCenter
                }
                background: Rectangle { color: "transparent" }
                onClicked: registerPageRoot.close()
            }
        }
    }
}
