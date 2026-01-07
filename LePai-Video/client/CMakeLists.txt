cmake_minimum_required(VERSION 3.28)
project(LePaiClient VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Quick Multimedia Network REQUIRED)

qt_standard_project_setup(REQUIRES 6.9)

qt_add_executable(LePaiClientApp)

qt_add_qml_module(LePaiClientApp
    URI LePaiClient
    VERSION 1.0
    SOURCES
        src/main.cpp
        src/ConfigManager.h
        src/ConfigManager.cpp
        src/model/AuthManager.h
        src/model/AuthManager.cpp
    QML_FILES
        src/view/Main.qml
)

target_include_directories(LePaiClientApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model
)

target_link_libraries(LePaiClientApp PRIVATE
    Qt6::Quick
    Qt6::Multimedia
    Qt6::Network
)

# 复制配置文件
set(CONFIG_SRC "${CMAKE_SOURCE_DIR}/config/config.json")
set(CONFIG_DST "${CMAKE_BINARY_DIR}/config.json")

add_custom_command(
    TARGET LePaiClientApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CONFIG_SRC}
        ${CONFIG_DST}
)