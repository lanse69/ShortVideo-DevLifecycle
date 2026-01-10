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
    void getGlobalFeed(long long limit, long long offset, FeedCallback callback);

    // 批量检查某用户是否点赞了这批视频 (读从库)
    void getLikedVideoIds(const std::string& userId, const std::vector<std::string>& videoIds, LikeStatusCallback callback);

    // 创建视频记录 (初始状态 status=0 处理中)
    void createVideo(const lepai::entity::Video& video, VoidCallback callback);

    // 添加点赞记录
    void addLikeRecord(const std::string& userId, const std::string& videoId, VoidCallback callback);

    // 移除点赞记录
    void removeLikeRecord(const std::string& userId, const std::string& videoId, VoidCallback callback);
    
    // 获取视频当前的基准点赞数 (用于缓存未命中时初始化 Redis)
    void getVideoLikeCount(const std::string& videoId, std::function<void(long long)> callback);

    // 获取关注视频流
    void getFollowingFeed(const std::string& userId, long long limit, long long offset, FeedCallback callback);

    // 点赞视频流 (查看 targetUserId 点赞过的视频)
    void getLikedFeed(const std::string& targetUserId, long long limit, long long offset, FeedCallback callback);

    // 用户发布列表 (作品页)
    void getUserUploadFeed(const std::string& targetUserId, long long limit, long long offset, FeedCallback callback);
};

}
}