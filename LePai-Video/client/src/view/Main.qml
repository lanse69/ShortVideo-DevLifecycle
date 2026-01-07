import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LePaiClient

Window {
    width: 360
    height: 640
    visible: true
    title: qsTr("LePai Vision - Register")

    AuthManager {
        id: authManager
        
        onRegistrationSuccess: {
            statusLabel.color = "green"
            statusLabel.text = "注册成功！"
            console.log("Register OK")
        }
        
        onRegistrationFailed: (message) => {
            statusLabel.color = "red"
            statusLabel.text = "注册失败: " + message
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width * 0.8
        spacing: 20

        Text {
            text: "注册"
            font.pixelSize: 24
            Layout.alignment: Qt.AlignHCenter
        }

        TextField {
            id: usernameField
            placeholderText: "用户名"
            Layout.fillWidth: true
        }

        TextField {
            id: passwordField
            placeholderText: "密码"
            echoMode: TextInput.Password
            Layout.fillWidth: true
        }
        
        Label {
            id: statusLabel
            visible: text !== ""
            Layout.fillWidth: true
        }

        Button {
            text: "提交"
            Layout.fillWidth: true
            onClicked: {
                statusLabel.text = "提交中..."
                statusLabel.color = "blue"
                authManager.registerUser(usernameField.text, passwordField.text)
            }
        }
    }
}