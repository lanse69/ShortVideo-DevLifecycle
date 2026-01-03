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
                font.pixelSize: 20
            }
        }

        // 底部控制区域 - 使用z轴提高层级
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 150
            color: "#000000"
            z: 10  // 提高层级，确保在弹窗之上

            RowLayout {
                anchors.fill: parent
                spacing: 0

                // 左边填充
                Item {
                    Layout.fillWidth: true
                }

                // 选择音乐按钮
                Button {
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: 80
                    background: Rectangle {
                        color: "#666666"
                        radius: 40
                    }
                    contentItem: Text {
                        text: selectedMusic ? "已选音乐" : "选择音乐"
                        color: "white"
                        font.pixelSize: 20
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: musicSelectorVisible = true
                }

                // 右边填充
                Item {
                    Layout.fillWidth: true
                }

                // 下一步按钮 - 修改：总可用
                Button {
                    Layout.preferredWidth: 200
                    Layout.preferredHeight: 80
                    background: Rectangle {
                        color: "#FF2C5C"  // 总是红色
                        radius: 40
                    }
                    contentItem: Text {
                        text: "下一步"
                        color: "white"
                        font.pixelSize: 20
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: {
                        // 不需要检查selectedMusic
                        root.editComplete(selectedMusic)
                    }
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
        z: 5  // 比控制区域低，但比背景高

        // 弹出动画
        y: musicSelectorVisible ? 0 : height

        Behavior on y {
            NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
        }

        // 点击弹窗外部关闭
        MouseArea {
            anchors.fill: parent
            onClicked: musicSelectorVisible = false
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            // 标题行 - 带关闭按钮
            RowLayout {
                Layout.fillWidth: true
                Layout.preferredHeight: 40

                Text {
                    text: "选择音乐"
                    color: "white"
                    font.pixelSize: 24
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true
                }

                // 关闭弹窗按钮
                Button {
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    text: "×"
                    background: Rectangle {
                        color: "transparent"
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.pixelSize: 24
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
                    spacing: 10
                }
            }

            // 确认按钮已删除
        }
    }

    // 音乐数据模型
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
            height: 80
            background: Rectangle {
                color: selected ? "#222222" : "#111111"
                radius: 10
                border.color: selected ? "#FF2C5C" : "transparent"
                border.width: selected ? 2 : 0
            }
            contentItem: RowLayout {
                anchors.fill: parent
                anchors.margins: 15

                // 音乐图标
                Rectangle {
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 50
                    radius: 8
                    color: "#333333"

                    Text {
                        anchors.centerIn: parent
                        text: "♪"
                        color: "white"
                        font.pixelSize: 24
                    }
                }

                // 音乐信息
                Column {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    spacing: 8

                    Text {
                        text: name
                        color: "white"
                        font.pixelSize: 16
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Text {
                        text: author + " · " + duration
                        color: "#AAAAAA"
                        font.pixelSize: 14
                    }
                }

                // 选择状态指示器
                Rectangle {
                    Layout.preferredWidth: 30
                    Layout.preferredHeight: 30
                    radius: 15
                    color: selected ? "#FF2C5C" : "#555555"

                    Text {
                        anchors.centerIn: parent
                        text: selected ? "✓" : ""
                        color: "white"
                        font.pixelSize: 16
                        font.bold: true
                    }
                }
            }

            onClicked: {
                // 点击已选中的项则取消选择
                if (selected) {
                    // 清除当前项的选中状态
                    musicModel.setProperty(index, "selected", false)
                    // 清除选中的音乐数据
                    selectedMusic = null
                } else {
                    // 清除所有项的选中状态
                    for (var i = 0; i < musicModel.count; i++) {
                        musicModel.setProperty(i, "selected", false)
                    }
                    // 设置当前项为选中
                    musicModel.setProperty(index, "selected", true)
                    // 保存选中的音乐数据
                    selectedMusic = {
                        name: name,
                        author: author,
                        duration: duration
                    }
                }
                // 点击后不关闭弹窗
            }
        }
    }

    // 顶部返回按钮
    Button {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 20
        text: "← 返回"
        background: Rectangle {
            color: "transparent"
        }
        contentItem: Text {
            text: parent.text
            color: "white"
            font.pixelSize: 16
        }
        onClicked: root.backRequested()
    }
}
