#pragma once

#include <drogon/drogon.h>
#include <vector>
#include <string>
#include <functional>

#include "../entity/Video.h"

namespace lepai {
namespace repository {

class VideoRepository {
public:
    using FeedCallback = std::function<void(std::vector<lepai::entity::Video> videos, const std::string& error)>;
    using LikeStatusCallback = std::function<void(const std::vector<std::string>& likedVideoIds, const std::string& error)>;
    using VoidCallback = std::function<void(bool success, const std::string& error)>;


    // 获取全站最新/热门视频 (读从库)
    void getGlobalFeed(int limit, int offset, FeedCallback callback);

    // 批量检查某用户是否点赞了这批视频 (读从库)
    void getLikedVideoIds(const std::string& userId, const std::vector<std::string>& videoIds, LikeStatusCallback callback);

    // 创建视频记录 (初始状态 status=0 处理中)
    void createVideo(const lepai::entity::Video& video, VoidCallback callback);
};

}
}