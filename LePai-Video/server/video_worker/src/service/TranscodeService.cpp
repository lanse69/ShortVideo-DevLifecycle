#include "TranscodeService.h"

#include <drogon/drogon.h>
#include <filesystem>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <atomic>
#include <iostream>

#include "../media/FFmpegHelper.h"
#include "config_manager.h"

namespace lepai {
namespace worker {
namespace service {

// 上传任务
struct UploadTask {
    std::string bucket;
    std::string objectKey;
    std::string localPath;
};

TranscodeService::TranscodeService() {
    queue = std::make_shared<messaging::TaskQueue>();
    repository = std::make_shared<repository::VideoStatusRepository>();
    
    auto& cfg = ConfigManager::instance();
    std::string minioEndpoint = cfg.getMinioHost() + ":" + std::to_string(cfg.getMinioPort());
    
    storage = std::make_unique<storage::MinioClient>(
        minioEndpoint,
        "lepai_minio", 
        "lepai_minio_pass"
    );
}

void TranscodeService::start() {
    LOG_INFO << "Transcode Service started.";
    // 注册轮询
    drogon::app().registerBeginningAdvice([this]() {
        drogon::app().getLoop()->runEvery(1.0, [this]() {
            queue->popTask([this](const std::string& videoId) {
                std::thread([this, videoId]() {
                    try {
                        this->processTask(videoId);
                    } catch (const std::exception& e) {
                        LOG_ERROR << "Task failed for " << videoId << ": " << e.what();
                        repository->markAsFailed(videoId);
                    }
                }).detach(); 

            });
        });
    });
}

void TranscodeService::processTask(const std::string& videoId) 
{
    LOG_INFO << "Starting pipeline for video: " << videoId;

    // 获取视频源地址 (MinIO Temp 桶的地址)
    std::string rawVideoUrl = repository->getVideoUrl(videoId);
    if (rawVideoUrl.empty()) {
        LOG_ERROR << "Video URL not found for id: " << videoId;
        repository->markAsFailed(videoId);
        return;
    }

    // 准备工作区
    // 本地临时根目录: /tmp/lepai_work/<videoId>/
    std::filesystem::path workDir = std::filesystem::path("/tmp/lepai_work") / videoId;
    std::filesystem::path coverPath = workDir / "cover.jpg";
    std::filesystem::path hlsDir = workDir / "hls"; // 存放切片文件的目录

    try {
        if (std::filesystem::exists(workDir)) std::filesystem::remove_all(workDir); // 清理旧数据
        std::filesystem::create_directories(hlsDir);
    } catch (const std::exception& e) {
        LOG_ERROR << "File system error: " << e.what();
        repository->markAsFailed(videoId);
        return;
    }

    auto& cfg = ConfigManager::instance();
    std::string cdnBaseUrl = "http://" + cfg.getCdnHost() + ":" + std::to_string(cfg.getCdnPort());
    std::string finalCoverUrl;
    std::string finalVideoUrl;
    int duration = 0;

    // 获取元数据 & 生成封面
    duration = media::FFmpegHelper::getVideoDuration(rawVideoUrl);
    if (media::FFmpegHelper::generateThumbnail(rawVideoUrl, coverPath.string())) {
        // 上传封面: public/covers/<videoId>.jpg
        std::string objectName = "covers/" + videoId + ".jpg";
        finalCoverUrl = storage->uploadFile("public", objectName, coverPath.string(), cdnBaseUrl);
    } else {
        // TODO: 生成/使用默认封面
        LOG_WARN << "Failed to generate thumbnail for video: " << videoId;
    }

    // 视频切片转码 (HLS)
    LOG_INFO << "Transcoding to HLS...";
    // 生成 outputDir/index.m3u8 和 outputDir/index_000.ts ...
    bool transcodeSuccess = media::FFmpegHelper::transcodeToHls(rawVideoUrl, hlsDir.string(), "index");
    
    if (!transcodeSuccess) {
        LOG_ERROR << "Transcode failed for " << videoId;
        repository->markAsFailed(videoId);
        std::filesystem::remove_all(workDir);
        return;
    }

    // 并发上传切片文件
    // 所有切片放在 MinIO 的 public/videos/<videoId>/ 目录下
    // 最终的 m3u8 地址就是: http://CDN/public/videos/<videoId>/index.m3u8
    std::vector<UploadTask> tasks;
    std::string minioPrefix = "videos/" + videoId;
    std::string m3u8ObjectName;

    // 扫描文件，构建任务列表
    for (const auto& entry : std::filesystem::directory_iterator(hlsDir)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            UploadTask task;
            task.bucket = "public";
            task.objectKey = minioPrefix + "/" + filename;
            task.localPath = entry.path().string();
            tasks.push_back(task);

            // 记录 m3u8 的路径
            if (entry.path().extension() == ".m3u8") {
                m3u8ObjectName = task.objectKey;
            }
        }
    }

    if (tasks.empty() || m3u8ObjectName.empty()) {
        LOG_ERROR << "No HLS files found or m3u8 missing.";
        repository->markAsFailed(videoId);
        std::filesystem::remove_all(workDir);
        return;
    }

    // 启动并发上传
    // 定义并发数
    const int CONCURRENCY = 4; // 4 个线程
    std::mutex queueMutex;
    std::queue<UploadTask> taskQueue;
    for (const auto& t : tasks) taskQueue.push(t);

    std::atomic<bool> uploadError{false}; // 错误标记
    std::vector<std::thread> workers;

    LOG_INFO << "Uploading " << tasks.size() << " segments with " << CONCURRENCY << " threads...";

    for (int i = 0; i < CONCURRENCY; ++i) {
        workers.emplace_back([&]() {
            while (true) {
                UploadTask currentTask;
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    if (taskQueue.empty()) return; // 任务做完了，线程退出
                    if (uploadError) return;       // 其他线程出错了，退出
                    currentTask = taskQueue.front();
                    taskQueue.pop();
                }

                // 执行耗时的上传操作 (此时没有锁)
                std::string res = storage->uploadFile(currentTask.bucket, currentTask.objectKey, currentTask.localPath, cdnBaseUrl);
                
                if (res.empty()) {
                    LOG_ERROR << "Failed to upload segment: " << currentTask.objectKey;
                    uploadError = true;
                }
            }
        });
    }

    // 等待所有线程完成
    for (auto& t : workers) {
        if (t.joinable()) t.join();
    }

    // 状态更新
    if (!uploadError) {
        // 构造最终播放地址
        finalVideoUrl = cdnBaseUrl + "/" + "public" + "/" + m3u8ObjectName;
        
        LOG_INFO << "Success! Video URL: " << finalVideoUrl;
        repository->markAsPublished(videoId, finalCoverUrl, duration, finalVideoUrl);
    } else {
        LOG_ERROR << "Upload failed due to network errors.";
        repository->markAsFailed(videoId);
    }

    // 清理
    try {
        std::filesystem::remove_all(workDir);
    } catch (...) {}
}

}
}
}