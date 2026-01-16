import QtQuick
import QtQuick.Layouts

// 关注了的视频内容区域
Rectangle {
    Layout.fillWidth: true
    Layout.fillHeight: true
    color: "#000000"
    property bool isFavorited: true
    Item {
        anchors.fill: parent

        // 单个视频项目
        Rectangle {
            anchors.fill: parent
            color: "#1a1a1a"

            // 视频信息区域 - 右侧工具栏
            ColumnLayout {
                id: rightToolBar
                anchors {
                    right: parent.right
                    rightMargin: 15
                    verticalCenter: parent.verticalCenter
                }
                spacing: 25

                // 头像
                ColumnLayout {
                    spacing: 0
                    Layout.alignment: Qt.AlignHCenter

                    // 头像
                    Rectangle {
                        Layout.preferredWidth: 50
                        Layout.preferredHeight: 50
                        Layout.alignment: Qt.AlignHCenter
                        radius: 25
                        color: "#87CEEB"
                        border.color: "#FFFFFF"
                        border.width: 2
                    }

                    // 关注按钮
                    Rectangle {
                        id: followButton
                        Layout.preferredWidth: 24
                        Layout.preferredHeight: 24
                        Layout.alignment: Qt.AlignHCenter
                        radius: 12
                        color: "#FF0050"
                        border.color: "#FFFFFF"
                        border.width: 2

                        // 状态：true=已关注（显示减号），false=未关注（显示加号）
                        property bool isFollowing: false

                        Text {
                            id: followText
                            text: followButton.isFollowing ? "-" : "+"
                            color: "#FFFFFF"
                            font.pixelSize: 14
                            font.bold: true
                            anchors.centerIn: parent
                        }

                        // 点击切换关注状态
                        TapHandler {
                            onTapped: {
                                followButton.isFollowing = !followButton.isFollowing
                                console.log(followButton.isFollowing ? "已关注" : "未关注")

                                // 添加点击动画
                                followAnimation.start()
                            }
                        }

                        // 关注/取消关注动画
                        SequentialAnimation {
                            id: followAnimation
                            running: false
                            ScaleAnimator {
                                target: followButton
                                from: 1.0
                                to: 1.2
                                duration: 100
                            }
                            ScaleAnimator {
                                target: followButton
                                from: 1.2
                                to: 1.0
                                duration: 100
                            }
                        }
                    }
                }

                // 喜欢按钮
                ColumnLayout {
                    spacing: 5
                    Layout.alignment: Qt.AlignHCenter

                    Image {
                        id: loveText
                        Layout.preferredWidth: 40
                        Layout.preferredHeight: 40
                        Layout.alignment: Qt.AlignHCenter

                        // 使用路径变量
                        property string likedPath: "qrc:/images/images/liked.png"
                        property string likePath: "qrc:/images/images/like.png"

                        // 初始显示红色实心
                        source: likedPath

                        // 点击事件
                        TapHandler {
                            onTapped: {
                                // 方法1：完整写法
                                var currentPath = loveText.source.toString()
                                var likedPath = "qrc:/images/images/liked.png"
                                var likePath = "qrc:/images/images/like.png"

                                loveText.source = (currentPath === likedPath) ? likePath : likedPath
                                lovefollowAnimation.start()
                            }
                        }
                        // 点赞/取消关注动画
                        SequentialAnimation {
                            id: lovefollowAnimation
                            running: false
                            ScaleAnimator {
                                target: loveText
                                from: 1.0
                                to: 1.2
                                duration: 100
                            }
                            ScaleAnimator {
                                target: loveText
                                from: 1.2
                                to: 1.0
                                duration: 100
                            }
                        }
                    }

                    Text {
                        text: "7.8w"
                        color: "#FFFFFF"
                        font.pixelSize: 12
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            // 底部描述区域
            ColumnLayout {
                anchors {
                    left: parent.left
                    leftMargin: 15
                    bottom: parent.bottom
                    bottomMargin: 30
                }
                spacing: 10

                // 用户名
                Text {
                    text: "@抖音用户"
                    color: "#FFFFFF"
                    font.pixelSize: 16
                    font.bold: true
                    Layout.maximumWidth: 280
                }

                // 视频描述
                Text {
                    text: "这是关注了的用户视频描述内容...#热门 #推荐"
                    color: "#FFFFFF"
                    font.pixelSize: 14
                    Layout.maximumWidth: 280
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    maximumLineCount: 2
                }
            }
        }
    }
}

