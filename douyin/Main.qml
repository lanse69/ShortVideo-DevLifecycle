import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: mainWindow

    // 判断是否为移动设备
    readonly property bool isMobile: Qt.platform.os === "android"

    // 直接设置尺寸
    width: isMobile ? Screen.width : 400   // 电脑固定宽度
    height: isMobile ? Screen.height : 800  // 电脑固定高度

    // 最小尺寸（防止窗口被缩得太小）
    minimumWidth: isMobile ? Screen.width : 400
    minimumHeight: isMobile ? Screen.height : 800


    // 最大尺寸（防止窗口被缩得太大）
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

    // Content Area
    Content {
        id: content
        anchors.fill: parent
    }
}
