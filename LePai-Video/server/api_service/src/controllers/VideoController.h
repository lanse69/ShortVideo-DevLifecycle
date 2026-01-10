#pragma once

#include <drogon/HttpController.h>

#include "../service/VideoService.h"

class VideoController : public drogon::HttpController<VideoController>
{
public:
    METHOD_LIST_BEGIN
    // POST /api/video/publish
    ADD_METHOD_TO(VideoController::publish, "/api/video/publish", drogon::Post, "LoginFilter");
    // 点赞/取消点赞
    // POST /api/video/like
    ADD_METHOD_TO(VideoController::toggleLike, "/api/video/like", drogon::Post, "LoginFilter");
    METHOD_LIST_END

    VideoController();

    void publish(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
    void toggleLike(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);

private:
    std::shared_ptr<lepai::service::VideoService> videoService;
};