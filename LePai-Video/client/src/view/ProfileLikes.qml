import QtQuick
import QtQuick.Layouts

Rectangle {
    id: contentArea
    Layout.fillWidth: true
    Layout.fillHeight: true  // 占据所有剩余空间
    color: "#111111"
    TapHandler {
            onTapped: {
                console.log("组件ID:", contentArea)
                console.log("实际宽度:", contentArea.width)
                console.log("实际高度:", contentArea.height)
            }
    }
    // 这里可以放置作品网格或其他内容
    Text {
        text: "喜欢内容区域"
        color: "#FFFFFF"
        font.pixelSize: 18
        anchors.centerIn: parent
    }
}
