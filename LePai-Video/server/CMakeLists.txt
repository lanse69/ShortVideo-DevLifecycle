cmake_minimum_required(VERSION 3.16)
project(LePaiServer VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 统一定义输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 全局依赖查找
find_package(Drogon REQUIRED)
find_package(PostgreSQL REQUIRED)
# find_package(OpenCV REQUIRED)

find_package(jsoncpp QUIET)
if(NOT jsoncpp_FOUND)
    find_package(JsonCpp QUIET) # 尝试驼峰命名
endif()
# 如果标准查找未定义 JsonCpp::JsonCpp 目标，则使用 PkgConfig
if(NOT TARGET JsonCpp::JsonCpp)
    message(STATUS "JsonCpp target not found via CMake config, trying PkgConfig...")
    find_package(PkgConfig REQUIRED)
    # 查找 jsoncpp 的 .pc 文件，并导入为 PkgConfig::JSONCPP 目标
    pkg_check_modules(JSONCPP REQUIRED IMPORTED_TARGET jsoncpp)
    # 创建一个别名 JsonCpp::JsonCpp 指向 PkgConfig 找到的目标
    add_library(JsonCpp::JsonCpp ALIAS PkgConfig::JSONCPP)
endif()

# 包含全局头文件路径
include_directories(${CMAKE_SOURCE_DIR}/common/include)

# 定义配置文件源路径和目标路径
set(CONFIG_SRC "${CMAKE_SOURCE_DIR}/config/config.json")
set(CONFIG_DST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json")

# 添加子模块
add_subdirectory(common)
add_subdirectory(api_service)
add_subdirectory(video_worker)