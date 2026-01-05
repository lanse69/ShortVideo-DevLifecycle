cmake_minimum_required(VERSION 4.0.3)

project(douyin VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.9) #set(CMAKE_AUTOUIC ON)  set(CMAKE_AUTOMOC ON)

qt_add_executable(appdouyin)

if(ANDROID)
    # 1. 应用信息（必须）
    set_target_properties(appdouyin PROPERTIES
        QT_ANDROID_APP_NAME "乐拍视界"
        QT_ANDROID_PACKAGE_NAME "com.baixiang.lepaishijie"
        # QT_ANDROID_APP_ICON "@drawable/icon"
    )

    # 2. 指定清单文件（必须）
    set_target_properties(appdouyin PROPERTIES
        QT_ANDROID_APP_MANIFEST "${CMAKE_CURRENT_SOURCE_DIR}/android/AndroidManifest.xml"
    )

    # 3. SDK版本（必须）
    set_target_properties(appdouyin PROPERTIES
        QT_ANDROID_MIN_SDK_VERSION "21"
        QT_ANDROID_TARGET_SDK_VERSION "33"
    )
endif()

qt_add_qml_module(appdouyin
    URI douyin
    VERSION 1.0
    SOURCES     main.cpp
    QML_FILES   Main.qml
    QML_FILES Content.qml
    QML_FILES BottomBar.qml
    QML_FILES HomeTab.qml
    QML_FILES HomeTopBar.qml
    QML_FILES HomeVideoArea.qml
    QML_FILES FriendsTab.qml
    QML_FILES FriendTopBar.qml
    QML_FILES FriendVideoArea.qml
    QML_FILES PostVideoTab.qml
    QML_FILES ProfileTab.qml
    QML_FILES ProfileLikes.qml
    QML_FILES ProfileWroks.qml
    QML_FILES HomeFollowPage.qml
    QML_FILES
    QML_FILES CaptureUploadPage.qml
    QML_FILES EditPage.qml
    QML_FILES PublishPage.qml
    RESOURCES icons.qrc
    RESOURCES android/gradle/wrapper/gradle-wrapper.jar android/gradle/wrapper/gradle-wrapper.properties android/gradle.properties android/gradlew android/gradlew.bat
    RESOURCES android/AndroidManifest.xml android/build.gradle android/res/values/libs.xml android/res/xml/qtprovider_paths.xml android/gradle/wrapper/gradle-wrapper.jar android/gradle/wrapper/gradle-wrapper.properties android/gradle.properties android/gradlew android/gradlew.bat
    RESOURCES android/AndroidManifest.xml android/build.gradle android/res/values/libs.xml android/res/xml/qtprovider_paths.xml android/gradle/wrapper/gradle-wrapper.jar android/gradle/wrapper/gradle-wrapper.properties android/gradle.properties android/gradlew android/gradlew.bat
    RESOURCES android/AndroidManifest.xml android/build.gradle android/res/values/libs.xml android/res/xml/qtprovider_paths.xml android/gradle/wrapper/gradle-wrapper.jar android/gradle/wrapper/gradle-wrapper.properties android/gradle.properties android/gradlew android/gradlew.bat
)

source_group("QML Files" FILES
    Main.qml Content.qml     QML_FILES BottomBar.qml
    QML_FILES HomeTab.qml
    QML_FILES HomeTopBar.qml
    QML_FILES HomeVideoArea.qml
    QML_FILES FriendsTab.qml
    QML_FILES FriendTopBar.qml
    QML_FILES FriendVideoArea.qml
    QML_FILES PostVideoTab.qml
    QML_FILES ProfileTab.qml
    QML_FILES ProfileLikes.qml
    QML_FILES ProfileWroks.qml
    QML_FILES HomeFollowPage.qml
)

target_compile_features(appdouyin PRIVATE cxx_std_23)

target_link_libraries(appdouyin
    PRIVATE
        Qt6::Quick
)

set_target_properties(appdouyin PROPERTIES
    AUTORCC TRUE                              #  set(CMAKE_AUTORCC ON)
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appdouyin
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS appdouyin
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


















































# set(CMAKE_CXX_STANDARD_REQUIRED ON) # Globally preventing the fallback behaviour of C++ standard
#qt_add_executable(douyin)  //Creates and finalizes an application target of a platform-specific type.
#This command performs the following tasks:
#    1. Create a CMake target of the appropriate type for the target platform.
#    2. Link the target to the Qt::Core library.
#    3. Handle finalization of the CMake target.

# set(CMAKE_CXX_STANDARD_REQUIRED ON) # Globally preventing the fallback behaviour of C++ standard

# qt_standard_project_setup()  #set(CMAKE_AUTOUIC ON)  set(CMAKE_AUTOMOC ON)






# cmake_minimum_required(VERSION 3.16)
#
# project(douyin VERSION 0.1 LANGUAGES CXX)
#
# @if !true
# set(CMAKE_AUTOMOC ON)
# @endif
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
#
# find_package(Qt6 REQUIRED COMPONENTS Quick)
#
# @if true
# @if true
# qt_standard_project_setup(REQUIRES 6.9)
# @else
# qt_standard_project_setup()
# @endif
# @endif
#
# qt_add_executable(appdouyin
#     main.cpp
# )
#
# qt_add_qml_module(appdouyin
#     URI douyin
#     VERSION 1.0
#     QML_FILES
#         Main.qml
# )
#
# # Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# # If you are developing for iOS or macOS you should consider setting an
# # explicit, fixed bundle identifier manually though.
# set_target_properties(appdouyin PROPERTIES
# #    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appdouyin
#     MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#     MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
#     MACOSX_BUNDLE TRUE
#     WIN32_EXECUTABLE TRUE
# )
#
# target_link_libraries(appdouyin
#     PRIVATE Qt6::Quick
# )
#
# include(GNUInstallDirs)
# install(TARGETS appdouyin
#     BUNDLE DESTINATION .
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )
