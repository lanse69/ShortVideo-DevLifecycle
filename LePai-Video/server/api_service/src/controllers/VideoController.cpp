#include "VideoController.h"

VideoController::VideoController() {
    videoService = std::make_shared<lepai::service::VideoService>();
}

// // 异步鉴权
// void VideoController::validateUser(const drogon::HttpRequestPtr& req, std::function<void(const std::string& userId)> validCallback, std::function<void(const drogon::HttpResponsePtr& resp)> invalidCallback) 
// {
//     // 提取 Token
//     std::string token = req->getHeader("Authorization");
//     if (token.size() > 7 && token.substr(0, 7) == "Bearer ") {
//         token = token.substr(7);
//     }
    
//     // 如果 Token 为空，直接返回 401
//     if (token.empty()) {
//         auto resp = drogon::HttpResponse::newHttpResponse();
//         resp->setStatusCode(drogon::k401Unauthorized);
//         resp->setBody("No token provided");
//         invalidCallback(resp);
//         return;
//     }

//     // 查询 Redis
//     auto redis = drogon::app().getRedisClient();
//     if (!redis) {
//         auto resp = drogon::HttpResponse::newHttpResponse();
//         resp->setStatusCode(drogon::k500InternalServerError);
//         resp->setBody("Redis service unavailable");
//         invalidCallback(resp);
//         return;
//     }

//     // 格式化 Key (session:token:UUID)
//     std::string key = "session:token:" + token;

//     redis->execCommandAsync(
//         [validCallback, invalidCallback](const drogon::nosql::RedisResult& r) {
//             // 找到了 Key (UserId)
//             if (r.type() == drogon::nosql::RedisResultType::kString) {
//                 std::string userId = r.asString();
//                 validCallback(userId); // 鉴权成功，把 userId 传回去
//             } else {
//                 // Key 不存在或过期
//                 auto resp = drogon::HttpResponse::newHttpResponse();
//                 resp->setStatusCode(drogon::k401Unauthorized);
//                 resp->setBody("Invalid or expired token");
//                 invalidCallback(resp);
//             }
//         },
//         [invalidCallback](const std::exception& e) {
//             LOG_ERROR << "Redis Auth Error: " << e.what();
//             auto resp = drogon::HttpResponse::newHttpResponse();
//             resp->setStatusCode(drogon::k500InternalServerError);
//             invalidCallback(resp);
//         },
//         "GET %s", key.c_str()
//     );
// }

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