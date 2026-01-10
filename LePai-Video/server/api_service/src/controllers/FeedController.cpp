#include "FeedController.h"

FeedController::FeedController() {
    recommendService = std::make_shared<lepai::service::RecommendationService>();
}

void FeedController::tryGetUserId(const drogon::HttpRequestPtr& req, std::function<void(std::string)> callback) 
{
    std::string token = req->getHeader("Authorization");
    if (token.size() > 7 && token.substr(0, 7) == "Bearer ") {
        token = token.substr(7);
    }

    if (token.empty()) {
        callback(""); // 游客
        return;
    }

    // 去 Redis 查 Token 有效性
    // TODO：封装在 AuthService 中
    auto redis = drogon::app().getRedisClient();
    std::string key = "session:token:" + token;
    
    redis->execCommandAsync(
        [callback](const drogon::nosql::RedisResult& r) {
            if (r.type() == drogon::nosql::RedisResultType::kString) {
                callback(r.asString()); // Token 有效，返回 UserId
            } else {
                callback(""); // Token 无效/过期，视为游客
            }
        },
        [callback](const std::exception&) {
            callback(""); // Redis 错误，降级为游客
        },
        "GET %s", key.c_str()
    );
}

void FeedController::getDiscoveryFeed(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    // 获取分页参数
    int limit = req->getParameter("limit").empty() ? 10 : std::stoi(req->getParameter("limit"));
    int offset = req->getParameter("offset").empty() ? 0 : std::stoi(req->getParameter("offset"));

    // 限制 limit 最大值
    if (limit > 50) limit = 50;

    // 尝试获取身份（异步）
    tryGetUserId(req, [this, limit, offset, callback](std::string userId) {
        recommendService->getDiscoveryFeed(userId, limit, offset, [callback, limit, offset](std::vector<lepai::entity::Video> videos, const std::string& err) {
            if (!err.empty()) {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k500InternalServerError);
                resp->setBody("Service Error");
                callback(resp);
                return;
            }

            // 构建响应
            Json::Value ret;
            ret["code"] = 200;
            ret["message"] = "success";
            
            Json::Value list(Json::arrayValue);
            for (const auto& v : videos) {
                list.append(v.toJson());
            }
            ret["data"] = list;

            int nextOffset;
            // 只要返回数量 < limit，就让客户端下一次重置为 0
            if (videos.size() < limit) {
                nextOffset = 0; // 下一次从头开始
            } else {
                nextOffset = offset + (int)videos.size(); // 正常翻页
            }

            ret["next_offset"] = nextOffset; 

            auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
            callback(resp);
        });
    });
}

void FeedController::getFollowingFeed(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    // 获取登录用户 ID (由 LoginFilter 注入)
    std::string userId = req->attributes()->get<std::string>("userId");
    
    // 获取分页参数
    int limit = req->getParameter("limit").empty() ? 10 : std::stoi(req->getParameter("limit"));
    int offset = req->getParameter("offset").empty() ? 0 : std::stoi(req->getParameter("offset"));

    if (limit > 50) limit = 50;

    recommendService->getFollowingFeed(userId, limit, offset, [callback, limit, offset](std::vector<lepai::entity::Video> videos, const std::string& err) {
        if (!err.empty()) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k500InternalServerError);
            resp->setBody("Service Error");
            callback(resp);
            return;
        }

        Json::Value ret;
        ret["code"] = 200;
        ret["message"] = "success";
        
        Json::Value list(Json::arrayValue);
        for (const auto& v : videos) {
            list.append(v.toJson());
        }
        ret["data"] = list;

        // 计算 next_offset
        int nextOffset;
        if (videos.size() < limit) {
            nextOffset = 0; // 没有更多数据了
        } else {
            nextOffset = offset + (int)videos.size();
        }
        ret["next_offset"] = nextOffset;

        callback(drogon::HttpResponse::newHttpJsonResponse(ret));
    });
}

// 获取点赞列表
void FeedController::getLikedFeed(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    tryGetUserId(req, [this, req, callback](std::string currentUserId) {
        
        std::string targetUserId = req->getParameter("user_id");

        if (targetUserId.empty()) {
            if (currentUserId.empty()) {
                // 既没传目标ID，自己又是游客 -> 报错
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k401Unauthorized);
                resp->setBody("Login required to view your own likes");
                callback(resp);
                return;
            }
            // 没传参数，默认为看自己
            targetUserId = currentUserId;
        }

        int limit = req->getParameter("limit").empty() ? 10 : std::stoi(req->getParameter("limit"));
        int offset = req->getParameter("offset").empty() ? 0 : std::stoi(req->getParameter("offset"));
        if (limit > 50) limit = 50;

        // targetUserId: 查谁的列表
        // currentUserId: 我是谁
        recommendService->getLikedFeed(targetUserId, currentUserId, limit, offset, [callback, limit, offset](std::vector<lepai::entity::Video> videos, const std::string& err) {
            if (!err.empty()) {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k500InternalServerError);
                resp->setBody("Service Error");
                callback(resp);
                return;
            }

            Json::Value ret;
            ret["code"] = 200;
            ret["message"] = "success";
            
            Json::Value list(Json::arrayValue);
            for (const auto& v : videos) {
                list.append(v.toJson());
            }
            ret["data"] = list;

            int nextOffset;
            if (videos.size() < limit) {
                nextOffset = 0;
            } else {
                nextOffset = offset + (int)videos.size();
            }
            ret["next_offset"] = nextOffset;

            callback(drogon::HttpResponse::newHttpJsonResponse(ret));
        });
    });
}

// 获取用户发布列表
void FeedController::getUserUploadFeed(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    // 尝试获取当前用户 ID
    tryGetUserId(req, [this, req, callback](std::string currentUserId) {
        
        std::string targetUserId = req->getParameter("user_id");

        if (targetUserId.empty()) {
            if (currentUserId.empty()) {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k401Unauthorized);
                resp->setBody("Login required to view your own uploads");
                callback(resp);
                return;
            }
            targetUserId = currentUserId;
        }

        int limit = req->getParameter("limit").empty() ? 10 : std::stoi(req->getParameter("limit"));
        int offset = req->getParameter("offset").empty() ? 0 : std::stoi(req->getParameter("offset"));
        if (limit > 50) limit = 50;

        recommendService->getUserUploadFeed(targetUserId, currentUserId, limit, offset, [callback, limit, offset](std::vector<lepai::entity::Video> videos, const std::string& err) {
            if (!err.empty()) {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k500InternalServerError);
                resp->setBody("Service Error");
                callback(resp);
                return;
            }

            Json::Value ret;
            ret["code"] = 200;
            ret["message"] = "success";
            
            Json::Value list(Json::arrayValue);
            for (const auto& v : videos) {
                list.append(v.toJson());
            }
            ret["data"] = list;

            int nextOffset;
            if (videos.size() < limit) {
                nextOffset = 0;
            } else {
                nextOffset = offset + (int)videos.size();
            }
            ret["next_offset"] = nextOffset;

            callback(drogon::HttpResponse::newHttpJsonResponse(ret));
        });
    });
}