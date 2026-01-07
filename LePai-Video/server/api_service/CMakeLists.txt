add_executable(api_service
    src/main.cpp
)

# 链接公共库和第三方库
target_link_libraries(api_service PRIVATE 
    lepai_common 
    pqxx 
    Qt6::Network 
    Qt6::Sql
)
