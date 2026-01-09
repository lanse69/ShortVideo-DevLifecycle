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

    /**
     * @brief 将视频转码并切片为 HLS (m3u8 + ts)
     * 
     * @param inputUrl 输入视频源地址
     * @param outputDir 输出目录
     * @param filePrefix 输出文件名的前缀
     * @return true 成功
     * @return false 失败
     */
    static bool transcodeToHls(const std::string& inputUrl, const std::string& outputDir, const std::string& filePrefix);
};

}
}
}