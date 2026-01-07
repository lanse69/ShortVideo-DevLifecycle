file(GLOB_RECURSE AS_SRC_FILES 
    "src/*.cpp"
    "src/*.h"
)

add_executable(api_service
    ${AS_SRC_FILES}
)

# 链接库
target_link_libraries(api_service PRIVATE 
    Drogon::Drogon
    lepai_common
    Qt6::Core 
    Qt6::Sql
)

add_custom_command(
    TARGET api_service POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CONFIG_SRC}
        ${CONFIG_DST}
)