#include "FFmpegHelper.h"

#include <drogon/drogon.h>
#include <cstdio>
#include <memory>
#include <array>
#include <cstdlib>

namespace lepai {
namespace worker {
namespace media {

// 执行 Shell 命令并获取输出
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, int(*)(FILE*)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int FFmpegHelper::getVideoDuration(const std::string& inputPath) {
    // 使用 ffprobe 获取时长
    // -v error: 只显示错误
    // -show_entries: 只显示时长
    // -of default=...: 格式化输出，只输出纯数字
    std::string cmd = "ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 \"" + inputPath + "\"";
    
    try {
        std::string out = exec(cmd.c_str());
        if (out.empty()) return 0;
        return std::stoi(out); // 转为整数秒
    } catch (const std::exception& e) {
        LOG_ERROR << "FFprobe failed: " << e.what();
        return 0;
    }
}

bool FFmpegHelper::generateThumbnail(const std::string& inputPath, const std::string& outputPath) {
    // 使用 ffmpeg 截图
    // -ss 00:00:01: 在第1秒截图
    // -i input: 输入
    // -vframes 1: 只输出1帧
    // -y: 覆盖输出文件
    std::string cmd = "ffmpeg -ss 00:00:01 -i \"" + inputPath + "\" -vframes 1 -q:v 2 -y \"" + outputPath + "\" > /dev/null 2>&1";
    
    int ret = std::system(cmd.c_str());
    return (ret == 0);
}

}
}
}