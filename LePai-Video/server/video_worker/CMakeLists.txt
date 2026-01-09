file(GLOB_RECURSE VW_SRC_FILES 
    "src/*.cpp"
    "src/*.h"
)

add_executable(video_worker
    ${VW_SRC_FILES}
)

target_link_libraries(video_worker PRIVATE 
    lepai_common
    Drogon::Drogon 
)

add_custom_command(
    TARGET video_worker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CONFIG_SRC}
        ${CONFIG_DST}
)