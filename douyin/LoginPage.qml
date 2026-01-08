import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: loginPage
    color: "#ffffff"
    radius: 20

    // 添加这两个信号
    signal loginSuccess()
    signal closeRequested()

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
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 25
        width: Math.min(parent.width * 0.85, 460)

        // 标题区域
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 12

            // 图标
            Image {
                Layout.alignment: Qt.AlignHCenter
                source: "qrc:/icon/icons/douyin.jpg"
                sourceSize: Qt.size(50, 50)
            }

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
                        }
                    }
                }

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
                            // 关键：设置初始不透明度，然后立即显示
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
                            anchors.rightMargin: 40  // 给眼睛留空间
                            leftPadding: 12
                            rightPadding: 40
                            font.pixelSize: 16
                            echoMode: TextField.Password
                            color: "#2c3e50"
                            placeholderText: "请输入密码"
                            placeholderTextColor: "#95a5a6"
                            background: Rectangle {
                                color: "transparent"
                            }
                            verticalAlignment: TextInput.AlignVCenter
                        }
                    }
                }
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
                            // 触发登录成功信号
                            loginSuccess()
                        }
              }

              // 错误提示
              Text {
                  id: errorText
                  Layout.alignment: Qt.AlignHCenter
                  color: "#e74c3c"
                  visible: false
                  font.pixelSize: 14
                  font.bold: true
              }

              // 底部提示 - 修改为实际提示
              Text {
                  Layout.alignment: Qt.AlignHCenter
                  Layout.topMargin: 5
                  text: "提示：请输入正确的账号和密码登录"
                  color: "#95a5a6"
                  font.pixelSize: 13
                }
            }
        }

        // 输入框获取焦点时清空错误提示
        Connections {
            target: usernameInput
            function onTextChanged() {
                errorText.visible = false
                loginButton.enabled = true
            }
        }

        Connections {
            target: passwordInput
            function onTextChanged() {
                errorText.visible = false
                loginButton.enabled = true
            }
        }
    }
}
