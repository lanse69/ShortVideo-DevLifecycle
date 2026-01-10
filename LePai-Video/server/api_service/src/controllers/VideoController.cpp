#include "VideoController.h"

VideoController::VideoController() {
    videoService = std::make_shared<lepai::service::VideoService>();
}

void VideoController::publish(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) 
{
    // 直接从 Attributes 获取 UserId
    std::string userId = req->attributes()->get<std::string>("userId");
            
    // 解析 JSON
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr || !jsonPtr->isMember("title") || !jsonPtr->isMember("url")) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody("Missing 'title' or 'url' in JSON body");
        callback(resp);
        return;
    }

    std::string title = (*jsonPtr)["title"].asString();
    std::string rawUrl = (*jsonPtr)["url"].asString();

    // 发布视频
    videoService->publishVideo(userId, title, rawUrl, [callback](bool success, const std::string& msg, const std::string& vid) {
        Json::Value ret;
        if (success) {
            ret["code"] = 200;
            ret["message"] = "Published successfully";
            ret["video_id"] = vid;
            callback(drogon::HttpResponse::newHttpJsonResponse(ret));
        } else {
            ret["code"] = 500;
            ret["message"] = msg;
            auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        }
    });
}

void VideoController::toggleLike(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback) 
{
    // 获取用户 ID
    std::string userId = req->attributes()->get<std::string>("userId");

    // 解析 JSON
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr || !jsonPtr->isMember("video_id") || !jsonPtr->isMember("action")) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody("Missing 'video_id' or 'action' (boolean)");
        callback(resp);
        return;
    }

    std::string videoId = (*jsonPtr)["video_id"].asString();
    bool action = (*jsonPtr)["action"].asBool(); // true: like, false: unlike

    videoService->toggleLike(userId, videoId, action, [callback](bool success, long long newCount, const std::string& msg) {
        Json::Value ret;
        if (success) {
            ret["code"] = 200;
            ret["message"] = msg;
            ret["like_count"] = (Json::Int64)newCount; // 返回最新点赞数给前端更新UI
        } else {
            // 区分是重复点赞还是系统错误
            if (msg == "Already liked" || msg == "Like record not found") {
                ret["code"] = 409; // Conflict
            } else {
                ret["code"] = 500;
            }
            ret["message"] = msg;
        }
        
        auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
        if (!success && ret["code"] == 500) resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
    });
}