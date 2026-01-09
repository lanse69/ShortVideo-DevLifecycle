#pragma once

#include <drogon/HttpController.h>

#include "../service/VideoService.h"

class VideoController : public drogon::HttpController<VideoController>
{
public:
    METHOD_LIST_BEGIN
    // POST /api/video/publish
    ADD_METHOD_TO(VideoController::publish, "/api/video/publish", drogon::Post);
    METHOD_LIST_END

    VideoController();

    void publish(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);

private:
    std::shared_ptr<lepai::service::VideoService> videoService;
    
    // 异步鉴权
    // 成功时调用 validCallback，失败时调用 invalidCallback (返回 401 或 500)
    void validateUser(const drogon::HttpRequestPtr& req, std::function<void(const std::string& userId)> validCallback, std::function<void(const drogon::HttpResponsePtr& resp)> invalidCallback
    );
};