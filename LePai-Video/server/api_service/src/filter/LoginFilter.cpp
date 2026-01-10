#include "LoginFilter.h"

#include <drogon/drogon.h>

void LoginFilter::doFilter(const drogon::HttpRequestPtr &req, drogon::FilterCallback &&fcb, drogon::FilterChainCallback &&ccb)
{
    // 获取 Authorization 头
    std::string token = req->getHeader("Authorization");
    if (token.size() > 7 && token.substr(0, 7) == "Bearer ") {
        token = token.substr(7);
    }

    if (token.empty()) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k401Unauthorized);
        resp->setBody("Missing Authorization Header");
        fcb(resp);
        return;
    }

    // 异步查询 Redis
    auto redis = drogon::app().getRedisClient();
    std::string key = "session:token:" + token;

    redis->execCommandAsync(
        [req, ccb, fcb](const drogon::nosql::RedisResult &r) {
            if (r.type() == drogon::nosql::RedisResultType::kString) {
                // [认证成功]
                std::string userId = r.asString();
                
                // 注入 userId
                req->attributes()->insert("userId", userId);
                
                // 放行
                ccb();
            } else {
                // [认证失败]
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->setStatusCode(drogon::k401Unauthorized);
                resp->setBody("Invalid or Expired Token");
                fcb(resp);
            }
        },
        [fcb](const std::exception &e) {
            LOG_ERROR << "Redis error in filter: " << e.what();
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k500InternalServerError);
            fcb(resp);
        },
        "GET %s", key.c_str()
    );
}