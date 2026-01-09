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
std::string exec(const char* cmd) 
{
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

int FFmpegHelper::getVideoDuration(const std::string& inputPath) 
{
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

bool FFmpegHelper::generateThumbnail(const std::string& inputPath, const std::string& outputPath) 
{
    // 使用 ffmpeg 截图
    // -ss 00:00:01: 在第1秒截图
    // -i input: 输入
    // -vframes 1: 只输出1帧
    // -y: 覆盖输出文件
    std::string cmd = "ffmpeg -ss 00:00:01 -i \"" + inputPath + "\" -vframes 1 -q:v 2 -y \"" + outputPath + "\" > /dev/null 2>&1";
    
    int ret = std::system(cmd.c_str());
    return (ret == 0);
}

bool FFmpegHelper::transcodeToHls(const std::string& inputUrl, const std::string& outputDir, const std::string& filePrefix) 
{
    // 构造输出文件路径模板: outputDir/index.m3u8
    std::string playlistPath = outputDir + "/" + filePrefix + ".m3u8";
    std::string segmentPattern = outputDir + "/" + filePrefix + "_%03d.ts";

    // FFmpeg 命令详解:
    // -i inputUrl: 输入
    // -c:v libx264: 视频编码器使用 H.264
    // -c:a aac: 音频编码器
    // -strict -2: 允许使用 AAC
    // -f hls: 输出格式为 HLS
    // -hls_time 10: 每个切片约 10 秒
    // -hls_list_size 0: m3u8 包含所有切片 (点播模式)
    // -hls_segment_filename: 切片文件命名规则
    // -preset veryfast -crf 23 平衡速度和质量
    std::stringstream cmd;
    cmd << "ffmpeg -v error -i \"" << inputUrl << "\" "
        << "-c:v libx264 -preset veryfast -crf 23 "
        << "-c:a aac -b:a 128k "
        << "-f hls "
        << "-hls_time 10 "
        << "-hls_list_size 0 "
        << "-hls_segment_filename \"" << segmentPattern << "\" "
        << "\"" << playlistPath << "\" "
        << "> /dev/null 2>&1";

    LOG_INFO << "Executing Transcode: " << cmd.str();
    
    int ret = std::system(cmd.str().c_str());
    return (ret == 0);
}

}
}
}