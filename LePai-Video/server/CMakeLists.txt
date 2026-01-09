cmake_minimum_required(VERSION 3.28)
project(LePaiServer VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 统一定义输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 全局依赖查找
find_package(Drogon REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(jsoncpp REQUIRED)
# find_package(OpenCV REQUIRED)
# find_package(FFmpeg REQUIRED) # 需要配置 FindFFmpeg.cmake

# 包含全局头文件路径
include_directories(\${CMAKE_SOURCE_DIR}/include)

# 定义配置文件源路径和目标路径
set(CONFIG_SRC "${CMAKE_SOURCE_DIR}/config/config.json")
set(CONFIG_DST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/config.json")

# 添加子模块
add_subdirectory(common)
add_subdirectory(api_service)
add_subdirectory(video_worker)