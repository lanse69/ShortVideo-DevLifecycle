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