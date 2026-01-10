#include "SyncScheduler.h"

#include <drogon/drogon.h>
#include <vector>
#include <string>

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

    // 随机获取最多 100 个脏数据 ID
    redis->execCommandAsync(
        [redis, db](const drogon::nosql::RedisResult &r) {
            if (r.type() == drogon::nosql::RedisResultType::kNil || r.asArray().empty()) {
                return; // 没有需要同步的数据
            }

            auto videoIds = r.asArray();
            LOG_DEBUG << "[Sync] Found " << videoIds.size() << " dirty videos to sync...";

            for (const auto &item : videoIds) {
                std::string vid = item.asString();
                // 构造 Key
                std::string likeKey = "video:likes:" + vid;

                // 获取该视频当前的最新点赞数
                redis->execCommandAsync(
                    [db, redis, vid](const drogon::nosql::RedisResult &countResult) {
                        if (countResult.type() == drogon::nosql::RedisResultType::kInteger) {
                            long long likes = countResult.asInteger();
                            
                            db->execSqlAsync(
                                "UPDATE videos SET like_count = $1 WHERE id = $2",
                                [redis, vid, likes](const drogon::orm::Result &r){
                                    // 只有 DB 更新成功后，才从 Redis 脏集合中移除
                                    redis->execCommandAsync(
                                        [](const drogon::nosql::RedisResult&){},
                                        [](const std::exception&){},
                                        "SREM dirty_videos %s", vid.c_str()
                                    );
                                    LOG_TRACE << "[Sync] Synced video " << vid << " with " << likes << " likes.";
                                },
                                [vid](const drogon::orm::DrogonDbException &e){
                                    // 失败时不移除，等待下一次轮询重试
                                    LOG_ERROR << "[Sync Error] DB Update failed for " << vid << ": " << e.base().what();
                                },
                                likes, vid
                            );
                        } else {
                             LOG_WARN << "[Sync] Invalid redis key type for " << vid;
                        }
                    },
                    [vid](const std::exception &e){
                         LOG_ERROR << "[Sync Error] Redis GET failed for " << vid << ": " << e.what();
                    },
                    "GET %s", likeKey.c_str()
                );
            }
        },
        [](const std::exception &e) {
            LOG_ERROR << "[Sync Error] Redis SRANDMEMBER failed:" << e.what();
        },
        "SRANDMEMBER dirty_videos 100" 
    );
}

} // namespace scheduler
} // namespace lepai