add_executable(video_worker
    src/main.cpp
)

target_link_libraries(video_worker PRIVATE 
    lepai_common 
    Qt6::Core
)
