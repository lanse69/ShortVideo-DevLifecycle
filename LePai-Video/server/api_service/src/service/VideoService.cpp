#include "VideoService.h"

#include <drogon/drogon.h>

#include "utils.h"
#include "../entity/Video.h"

namespace lepai {
namespace service {

VideoService::VideoService() {
    videoRepo = std::make_shared<lepai::repository::VideoRepository>();
}

void VideoService::publishVideo(const std::string& userId, const std::string& title, const std::string& rawUrl, PublishCallback callback) {
    // 构造实体
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
        // 键名必须与 Worker 中的 "video_queue" 保持一致
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

}
}