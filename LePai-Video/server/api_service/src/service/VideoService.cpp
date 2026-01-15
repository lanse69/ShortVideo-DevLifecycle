#include "VideoService.h"

#include <drogon/drogon.h>

#include "utils.h"
#include "../entity/Video.h"

namespace lepai {
namespace service {

VideoService::VideoService() {
    videoRepo = std::make_shared<lepai::repository::VideoRepository>();
}

void VideoService::publishVideo(const std::string& userId, const std::string& title, const std::string& rawUrl, PublishCallback callback) 
{
    lepai::entity::Video v;
    v.id = Utils::generateUUID();
    v.userId = userId;
    v.title = title;
    v.url = rawUrl; // 此时存的是 Temp 桶的源地址，等 Worker 转码后再更新为 Public CDN 地址

    // 写入数据库
    videoRepo->createVideo(v, [v, callback](bool success, const std::string& err) {
        if (!success) {
            callback(false, "DB Error: " + err, "");
            return;
        }

        // 写入 Redis 队列
        auto redis = drogon::app().getRedisClient();
        redis->execCommandAsync(
            [callback, v](const drogon::nosql::RedisResult& r) {
                LOG_INFO << "Task pushed to queue for video: " << v.id;
                callback(true, "Published, processing started", v.id);
            },
            [callback](const std::exception& e) {
                LOG_ERROR << "Redis Push Error: " << e.what();
                // TODO: 补偿机制
                callback(false, "Internal Queue Error", "");
            },
            "LPUSH video_queue %s", v.id.c_str() 
            // LPUSH + Worker的 RPOP 构成队列
        );
    });
}

void VideoService::toggleLike(const std::string& userId, const std::string& videoId, bool action, std::function<void(bool, long long, const std::string&)> callback) 
{
    // 操作数据库的关系表 (Source of Truth)
    auto dbCallback = [this, videoId, action, callback](bool success, const std::string& err) {
        if (!success) {
            // 数据库操作失败
            callback(false, -1, err.empty() ? "Operation failed" : err);
            return;
        }

        // 数据库更新成功后，更新 Redis 计数
        int delta = action ? 1 : -1;
        updateRedisLikeCount(videoId, delta, [callback](long long newCount) {
            callback(true, newCount, "Success");
        });
    };

    if (action) {
        videoRepo->addLikeRecord(userId, videoId, dbCallback);
    } else {
        videoRepo->removeLikeRecord(userId, videoId, dbCallback);
    }
}

void VideoService::updateRedisLikeCount(const std::string& videoId, int delta, std::function<void(long long)> resultCallback)
{
    auto redis = drogon::app().getRedisClient();
    std::string key = "video:likes:" + videoId;

    redis->execCommandAsync(
        [this, redis, key, videoId, delta, resultCallback](const drogon::nosql::RedisResult& r) {
            if (r.type() != drogon::nosql::RedisResultType::kNil) {
                // Key 存在，直接增减
                redis->execCommandAsync(
                    [redis, videoId, resultCallback](const drogon::nosql::RedisResult& r2) {
                        long long val = r2.asInteger();
                        // 加入脏队列
                        redis->execCommandAsync(
                            [](const drogon::nosql::RedisResult&){}, [](const std::exception&){},
                            "SADD dirty_videos %s", videoId.c_str()
                        );
                        resultCallback(val);
                    },
                    [](const std::exception&){},
                    delta > 0 ? "INCR %s" : "DECR %s", key.c_str()
                );
            } else {
                // Key 不存在 (Cache Miss)，回源查 DB
                videoRepo->getVideoLikeCount(videoId, [this, redis, key, videoId, delta, resultCallback](long long dbCount) {
                    long long newCount = dbCount + delta;
                    if (newCount < 0) newCount = 0;

                    // 重建缓存并设置过期时间(24h)
                    redis->execCommandAsync(
                        [redis, videoId](const drogon::nosql::RedisResult&){
                             // 加入脏队列
                             redis->execCommandAsync(
                                [](const drogon::nosql::RedisResult&){}, [](const std::exception&){},
                                "SADD dirty_videos %s", videoId.c_str()
                            );
                        }, 
                        [](const std::exception&){},
                        "SETEX %s 86400 %lld", key.c_str(), newCount
                    );
                    
                    resultCallback(newCount);
                });
            }
        },
        [](const std::exception& e) { LOG_ERROR << "Redis Error: " << e.what(); },
        "GET %s", key.c_str()
    );
}

}
}