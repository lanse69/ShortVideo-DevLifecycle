#include "LikeController.h"

#include <drogon/nosql/RedisClient.h>
#include <drogon/HttpAppFramework.h>

void LikeController::likeVideo(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    std::string videoId = req->getParameter("videoId");
    if (videoId.empty()) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    auto redis = drogon::app().getRedisClient();

    // 原子递增点赞数 (INCR)
    std::string key = "video:likes:" + videoId;
    
    redis->execCommandAsync(
        [redis, videoId, callback](const drogon::nosql::RedisResult &r) {
            // 将视频 ID 加入“脏数据集合” (SADD)
            redis->execCommandAsync(
                [callback](const drogon::nosql::RedisResult &r2){
                    Json::Value ret;
                    ret["status"] = "ok";
                    ret["message"] = "Liked (Buffered in Redis)";
                    auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
                    callback(resp);
                },
                [](const std::exception &e){ /* 错误处理忽略 */ },
                "SADD dirty_videos %s", videoId.c_str()
            );
        },
        [callback](const std::exception &e) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },
        "INCR %s", key.c_str()
    );
}