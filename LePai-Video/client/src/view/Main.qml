import QtQuick
import QtQuick.Controls

ApplicationWindow {
    id: mainWindow

    // 判断是否为移动设备
    readonly property bool isMobile: Qt.platform.os === "android" ||
                                     Qt.platform.os === "ios"
                                     // 移除重复的 tvos，如果是手机应用

    // 移动设备：全屏
    // 桌面设备：竖屏9:16比例，占屏幕合适比例

    width: isMobile ?
           // 手机：全屏
           Screen.width :
           // 电脑：竖屏9:16，宽度不超过屏幕的60%
           Math.min(Screen.desktopAvailableWidth * 0.6,  // 最大宽度60%
                   Screen.desktopAvailableHeight * 0.9 * (9/16))  // 按高度计算宽度

    height: isMobile ?
            // 手机：全屏
            Screen.height :
            // 电脑：竖屏9:16，高度基于宽度计算
            width * (16/9)  // 注意：width已经在上面的表达式计算好了

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
