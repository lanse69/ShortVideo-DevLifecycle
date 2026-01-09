#include "VideoStatusRepository.h"

namespace lepai {
namespace worker {
namespace repository {

std::string VideoStatusRepository::getVideoUrl(const std::string& videoId) {
    auto db = drogon::app().getDbClient("default");
    
    std::string url;
    try {
        auto future = db->execSqlAsyncFuture("SELECT url FROM videos WHERE id = $1", videoId);
        auto result = future.get(); // 阻塞等待结果
        if (result.size() > 0) {
            url = result[0]["url"].as<std::string>();
        }
    } catch (const std::exception& e) {
        LOG_ERROR << "DB Query Error: " << e.what();
    }
    return url;
}

void VideoStatusRepository::markAsPublished(const std::string& videoId, const std::string& coverUrl, int duration) {
    auto db = drogon::app().getDbClient("default");
    if (!db) return;

    db->execSqlAsync(
        "UPDATE videos SET status = 1, cover_url = $1, duration = $2, created_at = NOW() WHERE id = $3",
        [videoId](const drogon::orm::Result &r) {
            LOG_INFO << "Video " << videoId << " published.";
        },
        [videoId](const drogon::orm::DrogonDbException &e) {
            LOG_ERROR << "DB Update failed: " << e.base().what();
        },
        coverUrl, duration, videoId
    );
}

}
}
}