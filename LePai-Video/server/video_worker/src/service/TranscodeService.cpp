#include "TranscodeService.h"

#include <drogon/drogon.h>
#include <filesystem>
#include <thread>

#include "../media/FFmpegHelper.h"
#include "config_manager.h"

namespace lepai {
namespace worker {
namespace service {

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
                        LOG_ERROR << "Task failed for video " << videoId << ": " << e.what();
                    }
                }).detach(); 

            });
        });
    });
}

void TranscodeService::processTask(const std::string& videoId) {
    LOG_INFO << "Starting pipeline for video: " << videoId;

    // 获取视频源地址
    std::string videoUrl = repository->getVideoUrl(videoId);
    if (videoUrl.empty()) {
        LOG_ERROR << "Video URL not found for id: " << videoId;
        return;
    }

    // 准备临时文件路径
    std::string tempCoverPath = "/tmp/" + videoId + "_cover.jpg";

    // [Media] 获取时长
    int duration = media::FFmpegHelper::getVideoDuration(videoUrl);
    LOG_INFO << "Detected duration: " << duration << "s";

    // [Media] 生成封面
    bool thumbSuccess = media::FFmpegHelper::generateThumbnail(videoUrl, tempCoverPath);
    
    std::string coverUrl;
    if (thumbSuccess) {
        auto& cfg = ConfigManager::instance();
        // 构造 CDN Base URL
        std::string cdnBaseUrl = "http://" + cfg.getCdnHost() + ":" + std::to_string(cfg.getCdnPort());

        // [Storage] 上传封面
        std::string objectName = "covers/" + videoId + ".jpg";
        
        coverUrl = storage->uploadFile("public", objectName, tempCoverPath, cdnBaseUrl);
        
        // 清理临时文件
        std::filesystem::remove(tempCoverPath);
    } else {
        LOG_ERROR << "Failed to generate thumbnail.";
        // TODO: 设置一张默认失败图
        // coverUrl = "http://pc3-ip/public/error.jpg"; 
    }

    // --- 占位符：此处应包含视频切片、转码等复杂逻辑 ---
    // // processVideoTranscoding(...)
    // ---------------------------------------------

    // [Repo] 更新数据库，发布视频
    if (!coverUrl.empty()) {
        repository->markAsPublished(videoId, coverUrl, duration);
    }
}

}
}
}