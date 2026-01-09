#pragma once

#include <drogon/drogon.h>
#include <string>

namespace lepai {
namespace worker {
namespace repository {

class VideoStatusRepository {
public:
    // 获取视频源地址
    std::string getVideoUrl(const std::string& videoId);

    // 成功发布
    void markAsPublished(const std::string& videoId, const std::string& coverUrl, int duration, const std::string& videoUrl);

    // 标记为失败 (Status = 2)
    void markAsFailed(const std::string& videoId);
};

}
}
}