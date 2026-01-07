# 创建库
add_library(lepai_common SHARED
    src/utils.cpp
    src/config_manager.cpp
)

# 链接依赖
target_link_libraries(lepai_common PRIVATE Qt6::Core)

# 对外暴露头文件路径
target_include_directories(lepai_common PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
