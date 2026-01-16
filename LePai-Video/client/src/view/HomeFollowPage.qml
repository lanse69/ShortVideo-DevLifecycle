import QtQuick
import QtQuick.Layouts
import LePaiClient
// 视频内容区域
Rectangle {
    Layout.fillWidth: true
    Layout.fillHeight: true
    color: "#000000"
    property bool isFavorited: true

    function getCurrentToken() {
        return authManager ? authManager.getToken() : ""
    }

    BrowseVideosModelView {
        id: browseVideosModelView
        onVideosLoaded: (videoList)=>{
            console.log("收到视频数据，数量:", videoList.length);
            // 将新视频添加到ListModel
            for (var i = 0; i < videoList.length; i++) {
                var video = videoList[i];
                listModel.append({
                    "videoId": video.id,            // 使用 id 字段
                    "title": video.title,
                    "url": video.url,
                    "coverUrl": video.coverUrl,
                    "description": video.title,
                    //"createdAt": video.createdAt,   // 为空
                    "authorName": video.authorName, // 作者名
                    "authorAvatar": video.authorAvatar ,// 作者头像
                    "isFollowed":video.isFollowed,
                    "authorId":video.authorId,
                    "likeCount": video.likeCount,
                    "isLiked" :video.isLiked
                });
                console.log("视频:", video);
            }
        }
        // onVideosRequestFailed:{
        //     console.log("加载视频失败:", errorMessage);
        // }
        onLikeStatusChanged: (videoId, isLiked, likeCount) => {
                console.log("点赞状态变化，视频:", videoId, "点赞状态:", isLiked, "点赞数:", likeCount);

                // 更新 ListModel 中对应的视频项
                for (var i = 0; i < listModel.count; i++) {
                    if (listModel.get(i).videoId === videoId) {
                        listModel.setProperty(i, "isLiked", isLiked);
                        listModel.setProperty(i, "likeCount", likeCount);
                        console.log("更新视频", videoId, "点赞数:", likeCount, "点赞状态:", isLiked);
                    }
                }
            }
        onLikeFailed: (videoId, errorMessage) => {
            console.log("点赞失败，视频:", videoId, "错误:", errorMessage);
        }

        onFollowStatusChanged: (authorId, isFollowed) => {
            // 更新UI中该作者的所有视频的跟随状态
            for (var i = 0; i < listModel.count; i++) {
                if (listModel.get(i).authorId === authorId) {
                    listModel.setProperty(i, "isFollowed", isFollowed);
                }
            }
            refreshUserInfoTimer.start();
        }

        onFollowFailed: (authorId, errorMessage) => {
            console.log("关注失败，作者:", authorId, "错误:", errorMessage);
        }
    }

    Timer {
        id: initTimer
        interval: 10
        repeat: false
        onTriggered: {
             console.log("初始化加载视频，Token:", getCurrentToken())
             browseVideosModelView.requestFollowingVideos(getCurrentToken())
        }
    }

    Timer {
        id: refreshUserInfoTimer
        interval: 300 // 0.3秒
        repeat: false
        onTriggered: {
            console.log("开始刷新当前用户信息...");
            if (authManager && authManager.wasLogin) {
                authManager.refreshUserInfo();
            }
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
            if (contentY + height > contentHeight - 50) {
                browseVideosModelView.requestFollowingVideos(getCurrentToken());
            }
        }

        // 视频数据模型
        model:ListModel {
            id:listModel
            Component.onCompleted: {
                initTimer.start()
            }
        }

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
                            visible: model.authorId===authManager.currentUser.id? false:true

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
                                    var authorId = model.authorId;
                                    var currentFollowed = model.isFollowed;
                                    var newAction = !currentFollowed;

                                    var token = authManager.getToken();
                                    browseVideosModelView.followUser(authorId, newAction, token);

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

                        Image {
                            id: loveText
                            Layout.preferredWidth: 40
                            Layout.preferredHeight: 40
                            Layout.alignment: Qt.AlignHCenter

                            // 使用路径变量
                            property string likedPath: "qrc:/images/images/liked.png"
                            property string likePath: "qrc:/images/images/like.png"

                            // 初始显示红色实心
                            source: model.isLiked ? likedPath : likePath
                            opacity: videoItem.avatarOpacity

                            // 点击事件
                            TapHandler {
                                onTapped: {
                                    var videoId = model.videoId;
                                    var currentLiked = model.isLiked;
                                    var newAction = !currentLiked;  // 取反：点赞变取消，取消变点赞

                                    // 从 AuthManager 获取 token
                                    var token = authManager.getToken();

                                    // 调用点赞方法
                                    browseVideosModelView.likeVideo(videoId, newAction, token);

                                    // 添加点击动画
                                    lovefollowAnimation.start();

                                    console.log("点击点赞，视频ID:", videoId, "新状态:", newAction);
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
