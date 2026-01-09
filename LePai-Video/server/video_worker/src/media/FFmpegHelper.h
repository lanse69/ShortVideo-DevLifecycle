#pragma once

#include <string>
#include <optional>

namespace lepai {
namespace worker {
namespace media {

class FFmpegHelper {
public:
    // 获取视频时长（秒）
    // 支持本地路径或 HTTP URL
    static int getVideoDuration(const std::string& inputPath);

    // 截取视频封面
    // inputPath: 视频路径 (或 URL)
    // outputPath: 输出图片路径
    static bool generateThumbnail(const std::string& inputPath, const std::string& outputPath);
};

}
}
}