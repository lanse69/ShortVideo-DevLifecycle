#include "RecommendationService.h"

#include <drogon/drogon.h>

namespace lepai {
namespace service {

RecommendationService::RecommendationService() {
    videoRepo = std::make_shared<lepai::repository::VideoRepository>();
}

void RecommendationService::getDiscoveryFeed(const std::string& userId, int limit, int offset, lepai::repository::VideoRepository::FeedCallback callback) 
{
    // 策略：只有第一页(offset=0)走 Redis 缓存
    // 后续页走 DB
    if (offset > 0) {
        fetchFromDb(userId, limit, offset, callback);
        return;
    }

    auto redis = drogon::app().getRedisClient();
    std::string cacheKey = "feed:global:top";

    redis->execCommandAsync(
        [this, userId, limit, offset, callback, cacheKey](const drogon::nosql::RedisResult& r) {
            if (r.type() == drogon::nosql::RedisResultType::kString) {
                std::string jsonStr = r.asString();
                if (!jsonStr.empty()) {
                    // [Hit] 缓存命中
                    Json::Value root;
                    Json::Reader reader;
                    if (reader.parse(jsonStr, root) && root.isArray()) {
                        std::vector<lepai::entity::Video> videos;
                        for (const auto& item : root) {
                            videos.push_back(lepai::entity::Video::fromJson(item));
                        }
                        // 拿到基础数据后，填充个性化数据
                        enrichUserData(userId, videos, callback);
                        return;
                    }
                }
            }
            // [Miss] 缓存未命中
            fetchFromDb(userId, limit, offset, callback);
        },
        [this, userId, limit, offset, callback](const std::exception& e) {
            LOG_ERROR << "Redis Error: " << e.what();
            fetchFromDb(userId, limit, offset, callback); // 降级
        },
        "GET %s", cacheKey.c_str()
    );
}

void RecommendationService::fetchFromDb(const std::string& userId, int limit, int offset, lepai::repository::VideoRepository::FeedCallback callback) 
{
    videoRepo->getGlobalFeed(limit, offset, [this, userId, limit, offset, callback](std::vector<lepai::entity::Video> videos, const std::string& err) { 
        if (!err.empty()) {
            callback({}, err);
            return;
        }

        // 如果当前页没数据，且不是第一页，说明滑到底了。
        // 自动降级：重新从 offset = 0 开始查。
        if (videos.empty() && offset > 0) {
            LOG_INFO << "Feed reached end (offset " << offset << "), restarting from 0.";
            // 递归调用自己，但强制 offset 为 0
            fetchFromDb(userId, limit, 0, callback);
            return;
        }

        // 仅对第一页进行缓存回写 (Write Back)
        if (offset == 0 && !videos.empty()) {
            Json::Value arr(Json::arrayValue);
            for (const auto& v : videos) arr.append(v.toJson());
            Json::FastWriter writer;
            std::string jsonStr = writer.write(arr);

            // 缓存 15 秒，避免数据陈旧
            auto redis = drogon::app().getRedisClient();
            redis->execCommandAsync(
                [](const drogon::nosql::RedisResult&){}, 
                [](const std::exception&){}, 
                "SETEX feed:global:top 15 %s", jsonStr.c_str()
            );
        }

        enrichUserData(userId, videos, callback);
    });
}

void RecommendationService::enrichUserData(const std::string& userId, std::vector<lepai::entity::Video> videos, lepai::repository::VideoRepository::FeedCallback callback) 
{
    // 如果是游客，直接返回，不需要查点赞状态
    if (userId.empty() || videos.empty()) {
        callback(videos, "");
        return;
    }

    // 如果是登录用户，批量查这些视频的点赞状态
    std::vector<std::string> videoIds;
    for (const auto& v : videos) videoIds.push_back(v.id);

    videoRepo->getLikedVideoIds(userId, videoIds, [videos, callback](const std::vector<std::string>& likedIds, const std::string& err) mutable {
        if (err.empty() && !likedIds.empty()) {
            // 在内存中合并状态
            for (auto& v : videos) {
                for (const auto& likedId : likedIds) {
                    if (v.id == likedId) {
                        v.isLiked = true;
                        break;
                    }
                }
            }
        }
        // 无论点赞查询是否成功，都返回视频列表
        callback(videos, "");
    });
}

}
}