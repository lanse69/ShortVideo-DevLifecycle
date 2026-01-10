import QtQuick
import QtQuick.Layouts

Item {
    id: contentRoot

    // 当前底部Tab：0=首页，1=创建作品，2=个人空间
    property int currentTab: 0

    // 主列排列 - 整个页面垂直布局
    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // 内容区域 - 根据当前Tab显示不同内容
        Loader {
            id: contentLoader
            Layout.fillWidth: true
            Layout.fillHeight: true

            // 根据currentTab加载不同页面
            sourceComponent: {
                if (currentTab === 0) {
                    return _homeTab
                }
                // if (currentTab === 1) {
                //     return _friendTab
                // }
                if (currentTab === 1) {
                    return _postVideoTab
                }
                if (currentTab === 2) {
                    return _profileTab
                }
            }
        }

        // 底部标签栏
        BottomBar {
            id: _bottomBar
            Layout.fillWidth: true

            // 连接底部栏的信号
            onBottomButtonChanged: function(pageIndex) {
                // 切换其他Tab
                if (contentRoot.currentTab !== pageIndex) {
                    contentRoot.currentTab = pageIndex
                }
            }
        }
        // 首页页面
        Component {
            id: _homeTab
            HomeTab {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        // // 朋友页面
        // Component {
        //     id: _friendTab
        //     FriendsTab {
        //         Layout.fillWidth: true
        //         Layout.fillHeight: true
        //     }
        // }

        // 发视频页面
        Component {
            id: _postVideoTab
            PostVideoTab {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

        // 个人空间页面
        Component {
            id: _profileTab
            ProfileTab {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
}
