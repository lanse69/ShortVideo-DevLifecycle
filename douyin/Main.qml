import QtQuick
import QtQuick.Controls

ApplicationWindow {
    minimumWidth: height * (9 / 16)
    minimumHeight: Screen.desktopAvailableHeight * 0.8
    visible: true
    title: qsTr("乐拍视界")
    color:"black"
    //Content Area
    Content{
        id:content
        anchors.fill: parent
    }
}
