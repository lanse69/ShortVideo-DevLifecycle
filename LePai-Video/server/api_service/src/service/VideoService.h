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

    // 返回新的点赞数
    void toggleLike(const std::string& userId, const std::string& videoId, bool action, std::function<void(bool success, long long newCount, const std::string& msg)> callback);

private:
    std::shared_ptr<lepai::repository::VideoRepository> videoRepo;

    // 更新 Redis 缓存计数
    void updateRedisLikeCount(const std::string& videoId, int delta, std::function<void(long long)> resultCallback);
};

}
}