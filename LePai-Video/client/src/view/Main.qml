import QtQuick
import QtQuick.Controls
import LePaiClient

ApplicationWindow {
    id: mainWindow

    // 判断是否为移动设备
    readonly property bool isMobile: Qt.platform.os === "android"

    width: isMobile ? Screen.width : 400
    height: isMobile ? Screen.height : 800

    // 最小尺寸
    minimumWidth: isMobile ? Screen.width : 400
    minimumHeight: isMobile ? Screen.height : 800

    // 最大尺寸
    maximumWidth: isMobile ? Screen.width : 700
    maximumHeight: isMobile ? Screen.height : 1300
    visible: true
    title: qsTr("乐拍视界")
    color: "black"

    // 手机上启动后自动全屏
    Component.onCompleted: {
        if (isMobile) {
            showFullScreen()
        } else {
            // 桌面居中显示
            x = (Screen.width - width) / 2
            y = (Screen.height - height) / 2
        }
    }
    AuthManager{
        id:authManager
        property string loginMassage;
        property string registMassage;
        onRegistrationSuccess:{
            console.log("authManager:注册成功")
            registMassage="注册成功"
        }
        onRegistrationFailed:(message)=>{
            registMassage="注册失败"+message
            console.log("注册失败")
        }
        onLoginSuccess:{
            loginMassage="登陆成功"
            console.log(loginMassage)
            authManager.wasLogin=true
            console.log(authManager.wasLogin)
        }
        onLoginFailed:(message)=>{
            loginMassage=message
            console.log(loginMassage)
        }
    }

    Content{
        id:content
        anchors.fill: parent
    }
}
