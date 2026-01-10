import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import LePaiClient

Item {
    signal close()
    //背景
    Rectangle{
        radius: 20
        anchors.fill: parent
        color: "white"
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
           close()
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
            placeholderText: "请输入用户名"
            Layout.fillWidth: true
            placeholderTextColor: "#95a5a6"
        }

        TextField {
            id: passwordField
            Layout.fillWidth: true
            placeholderText: "请输入密码"
            placeholderTextColor: "#95a5a6"
        }

        Label {
            id: statusLabel
            visible: true
            Layout.fillWidth: true
            text:authManager.registMassage
        }

        Button {
            text: "注册"
            Layout.fillWidth: true
            onClicked: {
                statusLabel.color = "blue"
                authManager.registerUser(usernameField.text, passwordField.text)
            }
        }
    }
}
