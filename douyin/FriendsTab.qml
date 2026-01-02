import QtQuick
import QtQuick.Layouts

Item {
    //
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        FriendTopBar{
            id: _friendTopBar
            Layout.fillWidth: true
            Layout.preferredHeight: 50
        }
        FriendVideoArea{
            id: _friendVideoArea
        }
    }
}
