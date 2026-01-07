#include "SyncScheduler.h"

#include <drogon/drogon.h>
#include <QDebug>

void SyncScheduler::syncLikesToDB() {
    auto redis = drogon::app().getRedisClient();
    auto db = drogon::app().getDbClient("default"); // 用主库写入

    // 从 dirty_videos 集合中弹出所有 ID (SPOP count)
    redis->execCommandAsync(
        [redis, db](const drogon::nosql::RedisResult &r) {
            // 检查返回值类型是否为空
            if (r.type() == drogon::nosql::RedisResultType::kNil || r.asArray().empty()) {
                return; // 没有需要同步的数据
            }

            auto videoIds = r.asArray();
            qDebug() << "[Sync] Syncing" << videoIds.size() << "videos to DB...";

            for (const auto &item : videoIds) {
                std::string vid = item.asString();
                std::string key = "video:likes:" + vid;

                // 获取该视频当前的最新点赞数
                redis->execCommandAsync(
                    [db, vid](const drogon::nosql::RedisResult &countResult) {
                        if (countResult.type() == drogon::nosql::RedisResultType::kInteger) {
                            long long likes = countResult.asInteger();
                            
                            // 写入 PostgreSQL
                            db->execSqlAsync(
                                "UPDATE videos SET like_count = $1 WHERE id = $2",
                                [](const drogon::orm::Result &r){},
                                [](const drogon::orm::DrogonDbException &e){
                                    qCritical() << "[Sync Error]" << e.base().what();
                                },
                                likes, vid
                            );
                        }
                    },
                    [](const std::exception &e){
                         qCritical() << "[Sync Error] Failed to get likes count:" << e.what();
                    },
                    "GET %s", key.c_str()
                );
            }
        },
        [](const std::exception &e) {
            qCritical() << "[Sync Error] Failed to get dirty set:" << e.what();
        },
        "SPOP dirty_videos 100" 
    );
}