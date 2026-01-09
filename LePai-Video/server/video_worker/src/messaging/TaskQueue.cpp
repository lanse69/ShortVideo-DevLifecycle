#include "TaskQueue.h"

namespace lepai {
namespace worker {
namespace messaging {

void TaskQueue::popTask(TaskCallback callback) 
{
    auto redis = drogon::app().getRedisClient();
    if (!redis) return;

    redis->execCommandAsync(
        [callback](const drogon::nosql::RedisResult &r) {
            if (r.type() == drogon::nosql::RedisResultType::kString) {
                std::string videoId = r.asString();
                if (!videoId.empty()) {
                    callback(videoId);
                }
            }
        },
        [](const std::exception &e) {
            LOG_ERROR << "Redis RPOP Error: " << e.what();
        },
        "RPOP video_queue"
    );
}

}
}
}