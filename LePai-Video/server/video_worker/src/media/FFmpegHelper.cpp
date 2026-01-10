#include "FFmpegHelper.h"

#include <drogon/drogon.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <array>

namespace lepai {
namespace worker {
namespace media {

int FFmpegHelper::runProcess(const std::string& program, const std::vector<std::string>& args, std::string* output) 
{
    int pipefd[2];
    if (output && pipe(pipefd) == -1) {
        LOG_ERROR << "pipe() failed";
        return -1;
    }

    pid_t pid = fork();
    if (pid < 0) {
        LOG_ERROR << "fork() failed";
        return -1;
    }

    if (pid == 0) {
        // 子进程
        if (output) {
            close(pipefd[0]); // 关闭读端
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
        } else {
            int devNull = open("/dev/null", O_WRONLY);
            dup2(devNull, STDOUT_FILENO);
            dup2(devNull, STDERR_FILENO);
            close(devNull);
        }

        // 构造参数数组 char* const argv[]
        std::vector<char*> c_args;
        c_args.push_back(const_cast<char*>(program.c_str()));
        for (const auto& arg : args) {
            c_args.push_back(const_cast<char*>(arg.c_str()));
        }
        c_args.push_back(nullptr);

        // 执行命令
        execvp(program.c_str(), c_args.data());
        
        // 如果 execvp 返回，说明出错了
        perror("execvp failed");
        exit(1);
    } else {
        // 父进程
        int status;
        
        if (output) {
            close(pipefd[1]); // 关闭写端
            
            // 读取子进程输出
            std::array<char, 128> buffer;
            ssize_t bytesRead;
            while ((bytesRead = read(pipefd[0], buffer.data(), buffer.size())) > 0) {
                output->append(buffer.data(), bytesRead);
            }
            close(pipefd[0]);
        }

        // 等待子进程结束
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
}

int FFmpegHelper::getVideoDuration(const std::string& inputPath) 
{
    // 使用 ffprobe 获取时长
    // -v error: 只显示错误
    // -show_entries: 只显示时长
    // -of default=...: 格式化输出，只输出纯数字
    std::vector<std::string> args = {
        "-v", "error",
        "-show_entries", "format=duration",
        "-of", "default=noprint_wrappers=1:nokey=1",
        inputPath
    };

    std::string out;
    int ret = runProcess("ffprobe", args, &out);
    
    if (ret == 0 && !out.empty()) {
        try {
            return std::stoi(out);
        } catch (...) {
            return 0;
        }
    }
    return 0;
}

bool FFmpegHelper::generateThumbnail(const std::string& inputPath, const std::string& outputPath) 
{
    // 使用 ffmpeg 截图
    // -ss 00:00:01: 在第1秒截图
    // -i input: 输入
    // -vframes 1: 只输出1帧
    // -y: 覆盖输出文件
    std::vector<std::string> args = {
        "-ss", "00:00:01",
        "-i", inputPath,
        "-vframes", "1",
        "-q:v", "2",
        "-y",
        outputPath
    };

    int ret = runProcess("ffmpeg", args);
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
    // -f hls: 输出格式为 HLS
    // -hls_time 10: 每个切片约 10 秒
    // -hls_list_size 0: m3u8 包含所有切片 (点播模式)
    // -hls_segment_filename: 切片文件命名规则
    // -preset veryfast -crf 23 平衡速度和质量
    std::vector<std::string> args = {
        "-v", "error",
        "-i", inputUrl,
        "-c:v", "libx264",
        "-preset", "veryfast",
        "-crf", "23",
        "-c:a", "aac",
        "-b:a", "128k",
        "-f", "hls",
        "-hls_time", "10",
        "-hls_list_size", "0",
        "-hls_segment_filename", segmentPattern,
        playlistPath
    };

    LOG_INFO << "Starting HLS transcode...";
    int ret = runProcess("ffmpeg", args);
    return (ret == 0);
}

}
}
}