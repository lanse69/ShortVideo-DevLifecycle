import QtQuick
import QtQuick.Controls

ApplicationWindow {
    // width: Math.min(Screen.desktopAvailableWidth * 0.9, Screen.desktopAvailableHeight * 0.9 * (16/9))
    // height: Math.min(Screen.desktopAvailableHeight * 0.9, Screen.desktopAvailableWidth * 0.9 * (9/16))
    width: Math.min(Screen.desktopAvailableWidth , Screen.desktopAvailableHeight  * (16/9))
    height: Math.min(Screen.desktopAvailableHeight , Screen.desktopAvailableWidth * (9/16))
    visible: true
    title: qsTr("乐拍视界")
    color:"black"
    //Content Area
    Content{
        id:content
        anchors.fill: parent
    }
}
