import QtQuick
import QtQuick.Layouts
import LePaiClient
// 视频内容区域
Rectangle {
    Layout.fillWidth: true
    Layout.fillHeight: true
    color: "#000000"
    property bool isFavorited: true

    // Item {
    //     id: broseVideoViewModel

    //     function getVideos()
    //     {
    //          return [
    //             {
    //                 title:"zhoujun",
    //                 url:"file:///root/tmp/Linux Directories Explained in 100 Seconds.mp4",
    //                 description:"1111"
    //             },
    //             {
    //                 title :"zhu",
    //                 url :"file:///root/tmp/Linux Directories Explained in 100 Seconds.mp4",
    //                 description:"2"
    //             },
    //             {
    //                 title :"a",
    //                 url :"file:///root/tmp/Linux Directories Explained in 100 Seconds.mp4",
    //                 description:"3"
    //             },
    //         ]
    //     }
    // }


    BrowseVideosModelView {
        id: browseVideosModelView
        onVideosLoaded: (videoList)=>{
            console.log("收到视频数据，数量:", videoList.length);
            // 将新视频添加到ListModel
            for (var i = 0; i < videoList.length; i++) {
                var video = videoList[i];
                listModel.append({
                    "videoId": video.id,            // 使用 id 字段
                    //"userId": video.userId,         // 可能为空
                    "title": video.title,
                    "url": video.url,
                    "likeCount": video.likeCount,
                    "coverUrl": video.coverUrl,     // 使用 coverUrl 字段
                    "description": video.title,     // 描述可以用标题替代，因为没有description字段
                    //"createdAt": video.createdAt,   // 可能为空
                    "authorName": video.authorName, // 作者名
                    "authorAvatar": video.authorAvatar ,// 作者头像
                    "isFollowed":video.isFollowed,
                    "isLiked" :video.isLiked
                });
                console.log("视频:", video);
            }
        }
        onVideosRequestFailed:{
            console.log("加载视频失败:", errorMessage);
        }
    }


    ListView {
        id: videoListView
        anchors.fill: parent
        orientation: ListView.Vertical
        snapMode: ListView.SnapOneItem
        highlightRangeMode: ListView.StrictlyEnforceRange
        cacheBuffer: height * 2
        spacing: 2

        // 滑到底部提示
        property bool atBottomEnd: false
        onMovementEnded: {
            // // 检查是否滑到底部
            // if (contentY + height > contentHeight - 50) {
            //     listModel.getVideos()
            // }
            if (contentY + height > contentHeight - 50) {
                browseVideosModelView.requestVideos();
            }
        }

        // 视频数据模型
        model:ListModel {
            id:listModel
            Component.onCompleted: {
                browseVideosModelView.requestVideos();
            }
        }

        //     ListModel {
        //     id:listModel
        //     Component.onCompleted: {
        //         getVideos()
        //     }
        //     function getVideos(){
        //         var videos = broseVideoViewModel.getVideos()
        //         for(let i = 0; i < videos.length; i++) {
        //             append({
        //                        "title": videos[i].title,
        //                        "source": videos[i].url,
        //                        "description": videos[i].description
        //                    })
        //         }
        //     }
        // }

        delegate: Item {
            id: videoItem
            width: videoListView.width
            height: videoListView.height


            // 透明度控制
            property real avatarOpacity: 1.0
            property bool avatarPressed: false

            // 长按定时器
            Timer {
                id: longPressTimer
                interval: 500
                onTriggered: {
                    videoItem.avatarOpacity = 0.2
                    videoItem.avatarPressed = true
                }
            }

            // 单个视频项目
            Rectangle {
                id: videoBg
                anchors.fill: parent
                //color: bgColor
                VideoPlayWindow {
                    id: currentPlayer
                    width: videoListView.width
                    height: videoListView.height
                    playerSource:model.url
                    property bool shouldPlay: index==videoListView.currentIndex && videoListView.visible
                    onShouldPlayChanged: {
                        if (shouldPlay) {
                            mediaPlayer.play()
                        } else {
                           mediaPlayer.pause()
                        }
                    }
                }

                // 视频信息区域 - 右侧工具栏
                ColumnLayout {
                    id: rightToolBar
                    anchors {
                        right: videoBg.right
                        rightMargin: 15
                        verticalCenter: videoBg.verticalCenter
                    }
                    spacing: 25

                    // 头像
                    ColumnLayout {
                        id: avatarColumn
                        spacing: 0
                        Layout.alignment: Qt.AlignHCenter

                        // 头像
                        Rectangle {
                            id: avatarRect
                            Layout.preferredWidth: 50
                            Layout.preferredHeight: 50
                            Layout.alignment: Qt.AlignHCenter
                            radius: 25
                            color: "#FF0050"
                            border.color: "#FFFFFF"
                            border.width: 2
                            opacity: videoItem.avatarOpacity

                            // 长按事件
                            TapHandler {
                                onPressedChanged: {
                                    if (pressed) {
                                        longPressTimer.start()
                                    } else {
                                        longPressTimer.stop()
                                        if (videoItem.avatarPressed) {
                                            videoItem.avatarOpacity = 1.0
                                            videoItem.avatarPressed = false
                                        }
                                    }
                                }
                            }
                        }

                        // 关注按钮（紧靠在头像下面）
                        Rectangle {
                            id: followButton
                            Layout.preferredWidth: 24
                            Layout.preferredHeight: 24
                            Layout.alignment: Qt.AlignHCenter
                            radius: 12
                            color: "#FF0050"
                            border.color: "#FFFFFF"
                            border.width: 2
                            opacity: videoItem.avatarOpacity

                            // 状态：true=已关注（显示减号），false=未关注（显示加号）
                            property bool isFollowing: model.isFollowed

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

                    // 喜欢按钮（已激活）
                    ColumnLayout {
                        id: likeColumn
                        spacing: 5
                        Layout.alignment: Qt.AlignHCenter

                        Text {
                            id: loveText
                            text: "❤️"
                            color: "#FF0050"  // 红色表示已喜欢
                            font.pixelSize: 40
                            Layout.alignment: Qt.AlignHCenter
                            opacity: videoItem.avatarOpacity

                            // 点击事件
                            TapHandler {
                                onTapped: {
                                    if (loveText.text === "❤️"){ loveText.text = "🤍"}
                                        else loveText.text = "❤️"
                                    // 添加点击动画
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
                            id: likeCount
                            text: model.likeCount
                            color: "#FFFFFF"
                            font.pixelSize: 12
                            opacity: videoItem.avatarOpacity
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }

                // 底部描述区域
                ColumnLayout {
                    id: descColumn
                    anchors {
                        left: videoBg.left
                        leftMargin: 15
                        bottom: videoBg.bottom
                        bottomMargin: 30
                    }
                    spacing: 10

                    // 用户名
                    Text {
                        id: usernameText
                        text: model.authorName
                        color: "#FFFFFF"
                        font.pixelSize: 16
                        font.bold: true
                        Layout.maximumWidth: 280
                    }

                    // 视频描述
                    Text {
                        id: descriptionText
                        text: model.title
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

    // 底部提示框
    Rectangle {
        id: bottomTip
        anchors.centerIn: parent
        // anchors.bottom: parent.bottom
        // anchors.horizontalCenter: parent.horizontalCenter
        // anchors.bottomMargin: 30
        width: 120
        height: 40
        radius: 20
        color: "#FFFFFF"
        visible: false

        Text {
            anchors.centerIn: parent
            text: "已经到底啦！"
            color: "#000000"
            font.pixelSize: 14
        }
    }

    // 隐藏提示的定时器
    Timer {
        id: hideTimer
        interval: 2000
        onTriggered: bottomTip.visible = false
    }
}
