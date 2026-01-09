#include "SyncScheduler.h"

#include <drogon/drogon.h>

namespace lepai {
namespace scheduler {

void SyncScheduler::syncLikesToDB() 
{
    auto redis = drogon::app().getRedisClient();
    auto db = drogon::app().getDbClient("default"); // 用主库写入

    // 安全检查
    if (!redis) {
        LOG_WARN << "[Sync] Redis client not available.";
        return;
    }
    if (!db) {
        LOG_WARN << "[Sync] DB client 'default' not available.";
        return;
    }

    // 从 Redis 的脏数据集合中取出最多 100 个待同步的视频 ID
    redis->execCommandAsync(
        [redis, db](const drogon::nosql::RedisResult &r) {
            if (r.type() == drogon::nosql::RedisResultType::kNil || r.asArray().empty()) {
                return; // 没有需要同步的数据
            }

            auto videoIds = r.asArray();
            LOG_DEBUG << "[Sync] Syncing" << videoIds.size() << "videos to DB...";

            for (const auto &item : videoIds) {
                std::string vid = item.asString();
                std::string key = "video:likes:" + vid;

                // 获取该视频当前的最新点赞数
                redis->execCommandAsync(
                    [db, vid](const drogon::nosql::RedisResult &countResult) {
                        if (countResult.type() == drogon::nosql::RedisResultType::kInteger) {
                            long long likes = countResult.asInteger();
                            
                            // 异步写入 PostgreSQL
                            db->execSqlAsync(
                                "UPDATE videos SET like_count = $1 WHERE id = $2",
                                [](const drogon::orm::Result &r){},
                                [](const drogon::orm::DrogonDbException &e){
                                    LOG_ERROR << "[Sync Error] DB Update failed:" << e.base().what();
                                },
                                likes, vid
                            );
                        }
                    },
                    [](const std::exception &e){
                         LOG_ERROR << "[Sync Error] Redis GET failed:" << e.what();
                    },
                    "GET %s", key.c_str()
                );
            }
        },
        [](const std::exception &e) {
            LOG_ERROR << "[Sync Error] Redis SPOP failed:" << e.what();
        },
        "SPOP dirty_videos 100" 
    );
}

} // namespace scheduler
} // namespace lepai