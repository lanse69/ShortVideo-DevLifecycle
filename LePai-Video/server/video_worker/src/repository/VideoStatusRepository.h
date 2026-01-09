#pragma once

#include <drogon/drogon.h>
#include <string>
#include <optional>

namespace lepai {
namespace worker {
namespace repository {

class VideoStatusRepository {
public:
    // 获取视频源地址 (用于下载处理)
    std::string getVideoUrl(const std::string& videoId);

    // 更新状态
    void markAsPublished(const std::string& videoId, const std::string& coverUrl, int duration);
};

}
}
}