import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtMultimedia
import QtCore

Rectangle {
    id: root
    color: "#000000"

    signal videoSelected(string filePath)

    property bool isRecording: false
    property int cameraPosition: CameraDevice.BackFace

    MediaDevices {
        id: mediaDevices
    }

    CaptureSession {
        id: captureSession
        camera: Camera {
            id: camera
            cameraDevice: mediaDevices.defaultVideoInput
            active: true
            onErrorOccurred: (error, errorString) => console.error("相机错误:", errorString)
        }

        audioInput: AudioInput {
            id: audioInput
            muted: false // 确保录制声音
        }

        recorder: MediaRecorder {
            id: recorder
            onRecorderStateChanged: {
                if (recorderState === MediaRecorder.StoppedState) {
                    root.isRecording = false
                    console.log("录制完成，路径:", actualLocation)
                    
                    if (actualLocation !== "") {
                        root.videoSelected(actualLocation)
                    }
                } else if (recorderState === MediaRecorder.RecordingState) {
                    root.isRecording = true
                }
            }
            onErrorOccurred: (error, errorString) => {
                console.error("录制错误:", errorString)
                root.isRecording = false
            }
        }
        
        videoOutput: viewfinder
    }

    VideoOutput {
        id: viewfinder
        anchors.fill: parent
        fillMode: VideoOutput.PreserveAspectCrop
        
        // 前置摄像头时进行水平镜像
        transform: Scale {
            origin.x: viewfinder.width / 2
            xScale: root.cameraPosition === CameraDevice.FrontFace ? -1 : 1
        }
    }

    // 顶部操作栏
    Rectangle {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 60
        color: "transparent"
        visible: !root.isRecording // 录制时隐藏

        // 翻转摄像头按钮（右上角）
        Button {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.rightMargin: 20
            width: 80
            height: 40
            
            background: Rectangle {
                color: "#40000000"
                radius: 20
                border.color: "white"
                border.width: 1
            }
            
            contentItem: Text {
                text: "翻转"
                color: "white"
                font.pixelSize: 14
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            onClicked: toggleCamera()
        }
    }

    // 底部控制区域
    Rectangle {
        id: bottomControl
        anchors.bottom: parent.bottom
        width: parent.width
        height: 160
        
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 1.0; color: "#A0000000" }
        }

        Item {
            anchors.fill: parent
            
            // 左侧：上传按钮
            Button {
                id: uploadBtn
                width: 60
                height: 60
                anchors.left: parent.left
                anchors.leftMargin: 40
                anchors.verticalCenter: parent.verticalCenter
                visible: !root.isRecording // 录制时隐藏

                background: Rectangle {
                    color: "transparent"
                    border.color: "white"
                    border.width: 2
                    radius: 10
                }

                contentItem: Column {
                    anchors.centerIn: parent
                    spacing: 2
                    
                    Rectangle {
                        width: 20
                        height: 14
                        color: "white"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    
                    Text {
                        text: "上传"
                        color: "white"
                        font.pixelSize: 12
                        font.bold: true
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                }

                onClicked: fileDialog.open()
            }

            // 中间：拍摄按钮
            Item {
                id: recordBtnWrapper
                width: 90
                height: 90
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -10

                Rectangle {
                    anchors.fill: parent
                    radius: width / 2
                    color: "transparent"
                    border.color: "white"
                    border.width: 5
                    opacity: 0.8
                }

                Rectangle {
                    id: innerRecordBtn
                    anchors.centerIn: parent
                    width: root.isRecording ? 40 : 70
                    height: root.isRecording ? 40 : 70
                    radius: root.isRecording ? 6 : 35
                    color: "#FF2C5C"

                    Behavior on width { NumberAnimation { duration: 150; easing.type: Easing.InOutQuad } }
                    Behavior on height { NumberAnimation { duration: 150; easing.type: Easing.InOutQuad } }
                    Behavior on radius { NumberAnimation { duration: 150; easing.type: Easing.InOutQuad } }
                }

                TapHandler {
                    onTapped: {
                        if (root.isRecording) {
                            stopRecording()
                        } else {
                            startRecording()
                        }
                    }
                }
            }

            Text {
                anchors.bottom: recordBtnWrapper.top
                anchors.bottomMargin: 20
                anchors.horizontalCenter: parent.horizontalCenter
                text: "正在录制"
                color: "#FF2C5C"
                font.pixelSize: 16
                font.bold: true
                visible: root.isRecording
                
                SequentialAnimation on opacity {
                    running: root.isRecording
                    loops: Animation.Infinite
                    NumberAnimation { from: 1.0; to: 0.3; duration: 800 }
                    NumberAnimation { from: 0.3; to: 1.0; duration: 800 }
                }
            }
        }
    }

    // 文件选择
    FileDialog {
        id: fileDialog
        title: "选择视频"
        nameFilters: ["视频文件 (*.mp4 *.mov *.avi)", "所有文件 (*)"]
        onAccepted: {
            console.log("已选择本地文件:", currentFile)
            root.videoSelected(currentFile)
        }
    }

    function startRecording() {
        console.log("用户点击开始录制")
        recorder.record()
    }

    function stopRecording() {
        console.log("用户点击结束录制")
        recorder.stop()
    }

    // 切换前后摄像头逻辑
    function toggleCamera() {
        var inputs = mediaDevices.videoInputs
        if (inputs.length < 2) return

        var targetPos = (root.cameraPosition === CameraDevice.BackFace) 
                        ? CameraDevice.FrontFace 
                        : CameraDevice.BackFace
        
        var newDevice = null
        for (var i = 0; i < inputs.length; i++) {
            if (inputs[i].position === targetPos) {
                newDevice = inputs[i]
                break
            }
        }

        if (newDevice) {
            camera.active = false
            camera.cameraDevice = newDevice
            root.cameraPosition = targetPos
            camera.active = true
        }
    }
    
    // 页面销毁时关闭相机
    Component.onDestruction: {
        camera.active = false
    }
}