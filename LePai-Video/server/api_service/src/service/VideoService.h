#pragma once

#include <string>
#include <functional>
#include <memory>

#include "../repository/VideoRepository.h"

namespace lepai {
namespace service {

class VideoService {
public:
    using PublishCallback = std::function<void(bool success, const std::string& message, const std::string& videoId)>;

    VideoService();

    // 发布视频：userId, 标题, 原始视频地址(MinIO Temp地址)
    void publishVideo(const std::string& userId, const std::string& title, const std::string& rawUrl, PublishCallback callback);

private:
    std::shared_ptr<lepai::repository::VideoRepository> videoRepo;
};

}
}