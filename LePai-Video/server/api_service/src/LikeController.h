#pragma once

#include <drogon/HttpController.h>

class LikeController : public drogon::HttpController<LikeController>
{
public:
    METHOD_LIST_BEGIN
    // POST /api/video/like?videoId=123
    ADD_METHOD_TO(LikeController::likeVideo, "/api/video/like", drogon::Post);
    METHOD_LIST_END

    void likeVideo(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
};