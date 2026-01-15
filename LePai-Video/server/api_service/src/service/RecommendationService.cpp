#include "RecommendationService.h"

#include <drogon/drogon.h>
#include <algorithm>
#include <unordered_set>
#include <atomic>
#include <memory>
#include <sstream>

namespace lepai {
namespace service {

RecommendationService::RecommendationService() {
    videoRepo = std::make_shared<lepai::repository::VideoRepository>();
    userRepo = std::make_shared<lepai::repository::UserRepository>();
}

void RecommendationService::getDiscoveryFeed(const std::string& userId, int limit, int offset, lepai::repository::VideoRepository::FeedCallback callback) 
{
    // 只有第一页(offset=0)走 Redis 缓存
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
                        // 填充个性化数据
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

        // 如果当前页没数据，且不是第一页，则滑到底了
        if (videos.empty() && offset > 0) {
            LOG_INFO << "Feed reached end (offset " << offset << "), restarting from 0.";
            fetchFromDb(userId, limit, 0, callback);
            return;
        }

        // 仅对第一页进行缓存回写
        if (offset == 0 && !videos.empty()) {
            Json::Value arr(Json::arrayValue);
            for (const auto& v : videos) arr.append(v.toJson());
            Json::FastWriter writer;
            std::string jsonStr = writer.write(arr);

            // 缓存 15 秒
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
    if (videos.empty()) {
        callback(videos, "");
        return;
    }

    // 上下文结构
    struct EnrichContext {
        // 最终结果
        std::vector<lepai::entity::Video> finalVideos;
        
        // 结果缓存
        std::vector<std::string> likedVideoIds;      // 用户点赞过的视频ID
        std::vector<std::string> followingUserIds;   // 用户关注过的作者ID
        std::vector<long long> realTimeLikes;        // Redis中的实时点赞数 (-1表示Redis无记录)

        // 计数器：等待 3 个任务完成 (点赞状态、关注状态、实时计数)
        std::atomic<int> pendingTasks{3};
        
        // 回调
        lepai::repository::VideoRepository::FeedCallback doneCallback;
    };

    auto ctx = std::make_shared<EnrichContext>();
    ctx->finalVideos = std::move(videos); // 转移所有权
    ctx->doneCallback = callback;
    // 初始化实时点赞数为 -1
    ctx->realTimeLikes.resize(ctx->finalVideos.size(), -1);

    // 当所有任务完成时调用
    auto tryFinalize = [ctx]() {
        if (ctx->pendingTasks.fetch_sub(1) == 1) {
            
            std::unordered_set<std::string> likedSet(ctx->likedVideoIds.begin(), ctx->likedVideoIds.end());
            std::unordered_set<std::string> followingSet(ctx->followingUserIds.begin(), ctx->followingUserIds.end());

            for (size_t i = 0; i < ctx->finalVideos.size(); ++i) {
                auto& v = ctx->finalVideos[i];

                // 填充“已点赞”状态
                if (likedSet.count(v.id)) {
                    v.isLiked = true;
                }

                // 填充“已关注”状态
                if (followingSet.count(v.userId)) {
                    v.isFollowed = true;
                }
                else {
                    v.isFollowed = false;
                }

                // 修正点赞数
                if (ctx->realTimeLikes[i] >= 0) {
                    v.likeCount = ctx->realTimeLikes[i];
                }
            }

            ctx->doneCallback(ctx->finalVideos, "");
        }
    };

    // 查询“是否点赞”状态
    if (!userId.empty()) {
        std::vector<std::string> videoIds;
        videoIds.reserve(ctx->finalVideos.size());
        for (const auto& v : ctx->finalVideos) videoIds.push_back(v.id);

        videoRepo->getLikedVideoIds(userId, videoIds, [ctx, tryFinalize](const std::vector<std::string>& ids, const std::string&) {
            ctx->likedVideoIds = ids;
            tryFinalize();
        });
    } else {
        tryFinalize(); // 游客直接标记任务完成
    }

    // 查询“是否关注”状态
    if (!userId.empty()) {
        std::vector<std::string> authorIds;
        for (const auto& v : ctx->finalVideos) {
            // 排除空ID和自己
            if (!v.userId.empty() && v.userId != userId) {
                authorIds.push_back(v.userId);
            }
        }
        userRepo->getFollowingIds(userId, authorIds, [ctx, tryFinalize](const std::vector<std::string>& ids) {
            ctx->followingUserIds = ids;
            tryFinalize();
        });
    } else {
        tryFinalize();
    }

    // 查询 Redis 实时点赞数
    auto redis = drogon::app().getRedisClient();
    if (redis && !ctx->finalVideos.empty()) {
        std::string command = "MGET";
        for (const auto& v : ctx->finalVideos) {
            command += " video:likes:" + v.id;
        }

        redis->execCommandAsync(
            [ctx, tryFinalize](const drogon::nosql::RedisResult& r) {
                if (r.type() == drogon::nosql::RedisResultType::kArray) {
                    auto resArr = r.asArray();
                    for (size_t i = 0; i < resArr.size() && i < ctx->realTimeLikes.size(); ++i) {
                        if (resArr[i].type() != drogon::nosql::RedisResultType::kNil) {
                            ctx->realTimeLikes[i] = resArr[i].asInteger();
                        }
                    }
                }
                tryFinalize();
            },
            [tryFinalize](const std::exception& e) {
                LOG_ERROR << "Redis MGET Likes Error: " << e.what();
                tryFinalize(); // 降级显示 DB 数据
            },
            command.c_str() 
        );
    } else {
        tryFinalize();
    }
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