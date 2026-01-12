cmake_minimum_required(VERSION 3.28)
project(LePaiClient VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Quick Multimedia Network REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup(REQUIRES 6.9)

qt_add_executable(appLePaiClient)

qt_add_qml_module(appLePaiClient
    URI LePaiClient
    VERSION 1.0
    SOURCES
        src/main.cpp
        src/ConfigManager.h
        src/ConfigManager.cpp
        src/model/AuthManager.h
        src/model/AuthManager.cpp
        src/model/browsevideosmodelview.h
        src/model/browsevideosmodelview.cpp
        src/model/videomodel.h
        src/model/videomodel.cpp
        src/model/usermodel.h
        src/model/usermodel.cpp
        src/model/videopublisher.h
        src/model/videopublisher.cpp
        src/networkclient.h 
        src/networkclient.cpp
        src/model/fileutils.h 
        src/model/fileutils.cpp
        src/model/videoaudiomerger.h 
        src/model/videoaudiomerger.cpp

    QML_FILES
        src/view/Main.qml
        src/view/Content.qml
        src/view/BottomBar.qml
        src/view/HomeTab.qml
        src/view/HomeTopBar.qml
        src/view/HomeVideoArea.qml
        src/view/PostVideoTab.qml
        src/view/ProfileTab.qml
        src/view/ProfileLikes.qml
        src/view/ProfileWroks.qml
        src/view/HomeFollowPage.qml
        src/view/CaptureUploadPage.qml
        src/view/EditPage.qml
        src/view/PublishPage.qml
        src/view/VideoPlayWindow.qml
        src/view/LoginPage.qml
        src/view/Player.qml
        src/view/RegisterPage.qml
        src/view/AudioWaveform.qml
        src/view/MergeControlPanel.qml
        src/view/VideoWaveform.qml

    RESOURCES 
        android/AndroidManifest.xml 
        android/build.gradle 
        android/res/values/libs.xml 
        android/res/xml/qtprovider_paths.xml 
        android/gradle/wrapper/gradle-wrapper.jar 
        android/gradle/wrapper/gradle-wrapper.properties 
        android/gradle.properties 
        android/gradlew 
        android/gradlew.bat
)

target_include_directories(appLePaiClient PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model
)

target_link_libraries(appLePaiClient PRIVATE
    Qt6::Quick
    Qt6::Multimedia
    Qt6::Network
)
target_link_libraries(appLePaiClient PRIVATE Qt6::Core)

# 复制配置文件
set(CONFIG_SRC "${CMAKE_SOURCE_DIR}/config/config.json")
set(CONFIG_DST "${CMAKE_BINARY_DIR}/config.json")

add_custom_command(
    TARGET appLePaiClient POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CONFIG_SRC}
        ${CONFIG_DST}
)

if(ANDROID)
    set_property(TARGET appLePaiClient PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android)
endif()