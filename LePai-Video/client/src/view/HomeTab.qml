import QtQuick
import QtQuick.Layouts

ColumnLayout{
    Layout.fillWidth: true
    Layout.fillHeight: true
    property int currentPage: 0
    // 顶部导航栏
    HomeTopBar {
        id: _topBar
        Layout.fillWidth: true
        Layout.preferredHeight: 50

        // 连接TopBar的信号
        onPageChanged: function(pageIndex) {
            currentPage = pageIndex
        }
    }

    // 内容区域 - 使用Loader动态切换页面
    Loader {
        id: contentLoader
        Layout.fillWidth: true
        Layout.fillHeight: true

        // 根据currentPage加载不同页面
        sourceComponent: {
            if (currentPage === 0) {
                return videoAreaComponent
            } else {
                return followPageComponent
            }
        }
    }
    // 推荐页面组件
    Component {
        id: videoAreaComponent
        HomeVideoArea {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    // 关注页面组件
    Component {
        id: followPageComponent
        HomeFollowPage {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    // 同步TopBar的选中状态
    Connections {
        target: _topBar
        function onIsFollowTabChanged() {
            // 确保TopBar的状态和当前页面一致
            if (_topBar.isFollowTab && currentPage !== 1) {
                currentPage = 1
            } else if (!_topBar.isFollowTab && currentPage !== 0) {
                currentPage = 0
            }
        }
    }
}
