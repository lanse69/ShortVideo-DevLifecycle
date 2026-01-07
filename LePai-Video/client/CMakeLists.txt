cmake_minimum_required(VERSION 3.28)
project(LePaiClient VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Quick Multimedia Network REQUIRED)

qt_standard_project_setup(REQUIRES 6.9)

qt_add_qml_module(LePaiClient
    URI LePaiClient
    VERSION 1.0
    SOURCES
        src/main.cpp
    QML_FILES
        src/view/Main.qml
)

qt_add_executable(LePaiClient \${SOURCES} \${RESOURCE_FILES})

target_link_libraries(LePaiClient PRIVATE
    Qt6::Quick
    Qt6::Multimedia
    Qt6::Network
)
