#include "SessionRepository.h"

#include <drogon/drogon.h>

namespace lepai {
namespace repository {

void SessionRepository::saveSession(const std::string& userId, const std::string& token, long timeoutSeconds, VoidCallback callback) {
    auto redis = drogon::app().getRedisClient();
    
    // 存储当前有效的 token
    std::string userKey = "session:user:" + userId;
    // 存储 uid
    std::string tokenKey = "session:token:" + token;

    // 检查该用户是否已有登录会话
    redis->execCommandAsync(
        [redis, userKey, tokenKey, userId, token, timeoutSeconds, callback](const drogon::nosql::RedisResult& r) {
            if (r.type() == drogon::nosql::RedisResultType::kString) {
                std::string oldToken = r.asString();
                if (!oldToken.empty()) {
                    // 踢出旧的 token
                    std::string oldTokenKey = "session:token:" + oldToken;
                    redis->execCommandAsync(
                        [](const drogon::nosql::RedisResult&){},
                        [](const std::exception&){},
                        "DEL %s", oldTokenKey.c_str()
                    );
                    LOG_INFO << "Kicked out old token for user:" << userId;
                }
            }

            // 保存新会话
            redis->execCommandAsync(
                [redis, userKey, token, timeoutSeconds, callback](const drogon::nosql::RedisResult&) {
                    // 保存 Token -> User 映射
                    redis->execCommandAsync(
                        [callback](const drogon::nosql::RedisResult&) {
                            callback(true);
                        },
                        [callback](const std::exception&) { callback(false); },
                        "SETEX %s %ld %s", userKey.c_str(), timeoutSeconds, token.c_str() // 更新 User -> Token
                    );
                },
                [callback](const std::exception&) { callback(false); },
                "SETEX %s %ld %s", tokenKey.c_str(), timeoutSeconds, userId.c_str() // 更新 Token -> User
            );
        },
        [callback](const std::exception&) { callback(false); },
        "GET %s", userKey.c_str()
    );
}

void SessionRepository::removeSession(const std::string& token, VoidCallback callback) {
    auto redis = drogon::app().getRedisClient();
    std::string tokenKey = "session:token:" + token;

    // 获取 token 对应的 uid
    redis->execCommandAsync(
        [redis, tokenKey, callback](const drogon::nosql::RedisResult& r) {
            if (r.type() == drogon::nosql::RedisResultType::kString) {
                std::string uid = r.asString();
                std::string userKey = "session:user:" + uid;
                redis->execCommandAsync(
                    [](const drogon::nosql::RedisResult&){}, [](const std::exception&){},
                    "DEL %s", userKey.c_str()
                );
            }
            // 删除 Token
            redis->execCommandAsync(
                [callback](const drogon::nosql::RedisResult&){ callback(true); },
                [callback](const std::exception&){ callback(false); },
                "DEL %s", tokenKey.c_str()
            );
        },
        [callback](const std::exception&){ callback(false); },
        "GET %s", tokenKey.c_str()
    );
}

}
}