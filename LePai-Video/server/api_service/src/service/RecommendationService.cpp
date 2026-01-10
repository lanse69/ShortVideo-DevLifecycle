#include "RecommendationService.h"

#include <drogon/drogon.h>
#include <algorithm>
#include <unordered_set>
#include <atomic>
#include <memory>

namespace lepai {
namespace service {

RecommendationService::RecommendationService() {
    videoRepo = std::make_shared<lepai::repository::VideoRepository>();
    userRepo = std::make_shared<lepai::repository::UserRepository>();
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
    videoRepo->getGlobalFeed((long long)limit, (long long)offset, [this, userId, limit, offset, callback](std::vector<lepai::entity::Video> videos, const std::string& err) { 
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

// 填充个性化数据
void RecommendationService::enrichUserData(const std::string& userId, std::vector<lepai::entity::Video> videos, lepai::repository::VideoRepository::FeedCallback callback) 
{
    // 快速失败检查
    if (userId.empty() || videos.empty()) {
        callback(videos, "");
        return;
    }

    // 提取视频 ID 列表
    std::vector<std::string> videoIds;
    videoIds.reserve(videos.size());
    for (const auto& v : videos) videoIds.push_back(v.id);

    // 提取作者 ID 列表 (去重)
    std::vector<std::string> authorIds;
    authorIds.reserve(videos.size());
    for (const auto& v : videos) {
        if (!v.userId.empty() && v.userId != userId) {
            authorIds.push_back(v.userId);
        }
    }
    std::sort(authorIds.begin(), authorIds.end());
    auto last = std::unique(authorIds.begin(), authorIds.end());
    authorIds.erase(last, authorIds.end());

    // 定义共享上下文
    struct EnrichContext {
        // 结果容器
        std::vector<std::string> likedVideoIds;
        std::vector<std::string> followingUserIds;
        
        // 原始数据
        std::vector<lepai::entity::Video> finalVideos;
        
        // 计数器 (等待 2 个任务)
        std::atomic<int> pendingTasks{2};
        
        // 最终回调
        lepai::repository::VideoRepository::FeedCallback doneCallback;
    };

    auto ctx = std::make_shared<EnrichContext>(); // 使用 shared_ptr 保证上下文在所有回调结束前一直存活
    ctx->finalVideos = std::move(videos); // 转移所有权
    ctx->doneCallback = callback;

    // 定义合并逻辑
    auto tryFinalize = [ctx]() {
        // 只有当计数器减为 0 时，才执行合并
        // fetch_sub 返回减之前的值，所以如果返回 1，说明减完是 0
        if (ctx->pendingTasks.fetch_sub(1) == 1) {
            
            std::unordered_set<std::string> likedSet(ctx->likedVideoIds.begin(), ctx->likedVideoIds.end());
            std::unordered_set<std::string> followingSet(ctx->followingUserIds.begin(), ctx->followingUserIds.end());

            for (auto& v : ctx->finalVideos) {
                if (likedSet.count(v.id)) v.isLiked = true;
                if (followingSet.count(v.userId)) v.isFollowed = true;
                else v.isFollowed = false;
            }

            // 执行最终回调
            ctx->doneCallback(ctx->finalVideos, "");
        }
    };

    // 查询点赞
    videoRepo->getLikedVideoIds(userId, videoIds, [ctx, tryFinalize](const std::vector<std::string>& ids, const std::string&) {
        ctx->likedVideoIds = ids;
        tryFinalize();
    });

    // 查询关注
    userRepo->getFollowingIds(userId, authorIds, [ctx, tryFinalize](const std::vector<std::string>& ids) {
        ctx->followingUserIds = ids;
        tryFinalize();
    });
}

void RecommendationService::getFollowingFeed(const std::string& userId, int limit, int offset, lepai::repository::VideoRepository::FeedCallback callback) 
{
    videoRepo->getFollowingFeed(userId, (long long)limit, (long long)offset, [this, userId, callback](std::vector<lepai::entity::Video> videos, const std::string& err) {
        if (!err.empty()) {
            callback({}, err);
            return;
        }
        
        enrichUserData(userId, videos, callback);
    });
}

// 点赞列表
void RecommendationService::getLikedFeed(const std::string& targetUserId, const std::string& currentUserId, int limit, int offset, lepai::repository::VideoRepository::FeedCallback callback) 
{
    videoRepo->getLikedFeed(targetUserId, (long long)limit, (long long)offset, [this, currentUserId, callback](std::vector<lepai::entity::Video> videos, const std::string& err) {
        if (!err.empty()) {
            callback({}, err);
            return;
        }

        // 填充个性化状态
        enrichUserData(currentUserId, videos, callback);
    });
}

// 用户作品列表业务
void RecommendationService::getUserUploadFeed(const std::string& targetUserId, const std::string& currentUserId, int limit, int offset, lepai::repository::VideoRepository::FeedCallback callback) 
{
    videoRepo->getUserUploadFeed(targetUserId, (long long)limit, (long long)offset, [this, currentUserId, callback](std::vector<lepai::entity::Video> videos, const std::string& err) {
        if (!err.empty()) {
            callback({}, err);
            return;
        }

        // 填充个性化状态
        enrichUserData(currentUserId, videos, callback);
    });
}

}
}