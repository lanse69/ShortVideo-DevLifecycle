# 创建库
add_library(lepai_common STATIC
    src/utils.cpp
    src/config_manager.cpp
    src/MinioClient.cpp 
)

find_package(OpenSSL REQUIRED)

# 链接依赖
target_link_libraries(lepai_common PUBLIC 
    Drogon::Drogon
    JsonCpp::JsonCpp
    OpenSSL::SSL 
    OpenSSL::Crypto
)

# 对外暴露头文件路径
target_include_directories(lepai_common PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)