import QtQuick
import QtQuick.Layouts

// 底部标签栏
RowLayout {
    id: bottomBar
    Layout.preferredHeight: 60
    Layout.fillWidth: true
    spacing: 0

    // 信号：当页面切换时发出
    signal bottomButtonChanged(int pageIndex)  // 0=首页，1=朋友，2=创建作品，3=个人空间

    // 当前选中的按钮索引
    property int currentIndex: 0

    // 首页
    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 60
        color: "transparent"

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 4

            Text {
                text: "🏠"
                color: "#FF0050"
                font.pixelSize: 24
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "首页"
                color: bottomBar.currentIndex === 0 ? "#FF0050" : "#FFFFFF"
                font.pixelSize: 12
                Layout.alignment: Qt.AlignHCenter
            }
        }

        TapHandler {
            onTapped: {
                if (bottomBar.currentIndex !== 0) {
                    bottomBar.currentIndex = 0
                    bottomBar.bottomButtonChanged(0)
                }
            }
        }
    }

    // 发布按钮（特殊处理，不改变currentIndex）
    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 60
        color: "transparent"

        Text {
            text: "+"
            color: bottomBar.currentIndex === 1 ? "#FF0050" : "#FFFFFF"
            font.pixelSize: 30
            font.bold: true
            anchors.centerIn: parent
        }

        TapHandler {
            onTapped: {
                if (bottomBar.currentIndex !== 1) {
                    bottomBar.currentIndex = 1
                    bottomBar.bottomButtonChanged(1)
                }
            }
        }
    }

    // 我
    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: 60
        color: "transparent"

        ColumnLayout {
            anchors.centerIn: parent
            spacing: 4

            Text {
                text: "👤"
                color: "#FF0050"
                font.pixelSize: 24
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "我"
                color: bottomBar.currentIndex === 2 ? "#FF0050" : "#FFFFFF"
                font.pixelSize: 12
                Layout.alignment: Qt.AlignHCenter
            }
        }

        TapHandler {
            onTapped: {
                if (bottomBar.currentIndex !== 2) {
                    bottomBar.currentIndex = 2
                    bottomBar.bottomButtonChanged(2)
                }
            }
        }
    }
}
