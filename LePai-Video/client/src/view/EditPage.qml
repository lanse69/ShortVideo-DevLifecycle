import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "#000000"

    property string videoSource: ""
    signal editComplete(var musicData)
    signal backRequested

    property var selectedMusic: null
    property bool musicSelectorVisible: false

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 视频预览区域
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#111111"

            Text {
                anchors.centerIn: parent
                text: "视频预览区域"
                color: "white"
                font.pixelSize: parent.width * 0.05  // 使用 parent.width
            }
        }

        // 底部控制区域
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: parent.height * 0.25  // 使用 parent.height
            color: "#000000"
            z: 10

            RowLayout {
                anchors.fill: parent
                spacing: parent.width * 0.05  // 使用 parent.width

                // 左边填充
                Item {
                    Layout.fillWidth: true
                }

                // 选择音乐按钮
                Button {
                    Layout.preferredWidth: Math.min(parent.width * 0.4, 200)  // 限制最大宽度
                    Layout.preferredHeight: Math.min(parent.width * 0.12, 50)
                    background: Rectangle {
                        color: "#666666"
                        radius: height / 2
                    }
                    contentItem: Text {
                        text: selectedMusic ? "已选音乐" : "选择音乐"
                        color: "white"
                        font.pixelSize: Math.max(15, parent.width * 0.04)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        anchors.centerIn: parent
                    }
                    onClicked: musicSelectorVisible = true
                }

                // 中间填充
                Item {
                    Layout.fillWidth: true
                }

                // 下一步按钮
                Button {
                    Layout.preferredWidth: Math.min(parent.width * 0.4, 200)
                    Layout.preferredHeight: Math.min(parent.width * 0.12, 50)
                    background: Rectangle {
                        color: "#FF2C5C"
                        radius: height / 2
                    }
                    contentItem: Text {
                        text: "下一步"
                        color: "white"
                        font.pixelSize: Math.max(15, parent.width * 0.04)
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        anchors.centerIn: parent
                    }
                    onClicked: root.editComplete(selectedMusic)
                }

                // 右边填充
                Item {
                    Layout.fillWidth: true
                }
            }
        }
    }

    // 音乐选择弹出层 - 添加关闭按钮
    Rectangle {
        id: musicSelector
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: parent.height * 0.7
        color: "#1A1A1A"
        visible: musicSelectorVisible
        z: 5

        y: musicSelectorVisible ? 0 : height

        Behavior on y {
            NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: musicSelectorVisible = false
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Math.min(parent.width * 0.05, 20)
            spacing: Math.min(parent.width * 0.03, 10)

            // 标题行
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: Math.min(parent.width * 0.1, 40)

                Text {
                    text: "选择音乐"
                    color: "white"
                    font.pixelSize: Math.max(16, parent.width * 0.045)
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                }

                Button {
                    Layout.preferredWidth: Math.min(parent.width * 0.1, 30)
                    Layout.preferredHeight: Math.min(parent.width * 0.1, 30)
                    text: "×"
                    background: Rectangle {
                        color: "transparent"
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: Math.max(16, parent.width * 0.045)
                        font.bold: true
                    }
                    onClicked: musicSelectorVisible = false
                }
            }

            // 音乐列表
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                ListView {
                    id: musicListView
                    model: musicModel
                    delegate: musicItemDelegate
                    spacing: Math.min(parent.width * 0.02, 8)
                }
            }
        }
    }

    // 音乐数据模型（保持不变）
    ListModel {
        id: musicModel

        ListElement {
            name: "热门推荐 - 背景音乐1"
            author: "抖音音乐"
            duration: "30s"
            selected: false
        }
        ListElement {
            name: "轻快节奏 - 夏日海滩"
            author: "原创音乐人"
            duration: "45s"
            selected: false
        }
        ListElement {
            name: "情感配乐 - 回忆往事"
            author: "热门用户"
            duration: "60s"
            selected: false
        }
        ListElement {
            name: "节奏感强 - 舞蹈专用"
            author: "舞蹈达人"
            duration: "35s"
            selected: false
        }
        ListElement {
            name: "舒缓钢琴曲"
            author: "经典音乐"
            duration: "50s"
            selected: false
        }
    }

    // 音乐项委托
    Component {
        id: musicItemDelegate

        Button {
            width: musicListView.width
            height: Math.min(musicListView.width * 0.2, 80)
            background: Rectangle {
                color: selected ? "#222222" : "#111111"
                radius: Math.min(width * 0.05, 8)
                border.color: selected ? "#FF2C5C" : "transparent"
                border.width: 2
            }
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.margins: Math.min(parent.width * 0.03, 10)

                Rectangle {
                    Layout.preferredWidth: Math.min(parent.width * 0.12, 40)
                    Layout.preferredHeight: Math.min(parent.width * 0.12, 40)
                    radius: Math.min(width * 0.2, 6)
                    color: "#333333"

                    Text {
                        anchors.centerIn: parent
                        text: "♪"
                        color: "white"
                        font.pixelSize: Math.max(14, parent.width * 0.3)
                    }
                }

                Column {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: Math.min(parent.width * 0.01, 4)

                    Text {
                        text: name
                        color: "white"
                        font.pixelSize: Math.max(14, parent.width * 0.035)
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Text {
                        text: author + " · " + duration
                        color: "#AAAAAA"
                        font.pixelSize: Math.max(12, parent.width * 0.03)
                    }
                }

                Rectangle {
                    Layout.preferredWidth: Math.min(parent.width * 0.07, 25)
                    Layout.preferredHeight: Math.min(parent.width * 0.07, 25)
                    radius: width / 2
                    color: selected ? "#FF2C5C" : "#555555"

                    Text {
                        anchors.centerIn: parent
                        text: selected ? "✓" : ""
                        color: "white"
                        font.pixelSize: Math.max(12, parent.width * 0.3)
                        font.bold: true
                    }
                }
            }

            onClicked: {
                if (selected) {
                    musicModel.setProperty(index, "selected", false)
                    selectedMusic = null
                } else {
                    for (var i = 0; i < musicModel.count; i++) {
                        musicModel.setProperty(i, "selected", false)
                    }
                    musicModel.setProperty(index, "selected", true)
                    selectedMusic = {
                        name: name,
                        author: author,
                        duration: duration
                    }
                }
            }
        }
    }

    // 顶部返回按钮
    Button {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: Math.min(parent.width * 0.05, 20)
        text: "← 返回"
        background: Rectangle {
            color: "transparent"
        }
        contentItem: Text {
            text: parent.text
            color: "white"
            font.pixelSize: Math.max(14, parent.width * 0.04)
        }
        onClicked: root.backRequested()
    }
}
