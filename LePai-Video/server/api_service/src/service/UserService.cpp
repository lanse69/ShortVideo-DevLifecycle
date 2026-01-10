#include "UserService.h"

#include <drogon/drogon.h>
#include <json/json.h>

#include "utils.h"
#include "config_manager.h"

namespace lepai {
namespace service {

UserService::UserService() {
    userRepo = std::make_shared<lepai::repository::UserRepository>();
    sessionRepo = std::make_shared<lepai::repository::SessionRepository>();

    // 初始化 MinIO 客户端
    auto& cfg = ConfigManager::instance();
    std::string endpoint = cfg.getMinioHost() + ":" + std::to_string(cfg.getMinioPort());
    
    storageClient = std::make_unique<lepai::storage::MinioClient>(
        endpoint, 
        cfg.getMinioUser(),
        cfg.getMinioPassword()
    );
}

void UserService::registerUser(const std::string& username, const std::string& password, RegisterCallback callback) 
{
    // 检查用户是否存在
    userRepo->findByUsername(username, [this, username, password, callback](const std::optional<lepai::entity::User>& userOpt, const std::string& error) {
        if (!error.empty()) {
            callback(false, "System error during check");
            return;
        }
        if (userOpt.has_value()) {
            callback(false, "Username already exists");
            return;
        }

        // 准备实体
        lepai::entity::User newUser;
        newUser.id = Utils::generateUUID();
        newUser.username = username;
        newUser.passwordHash = Utils::hashPassword(password);

        // 写入数据库
        userRepo->createUser(newUser, [callback](bool success, const std::string& err) {
            if (success) {
                callback(true, "Registration successful");
            } else {
                LOG_ERROR << "Create user DB error:" << err;
                callback(false, "Registration failed");
            }
        });
    });
}

void UserService::login(const std::string& username, const std::string& password, LoginCallback callback) 
{
    // 查找用户
    userRepo->findByUsername(username, [this, password, callback](const std::optional<lepai::entity::User>& userOpt, const std::string& error) {
        if (!userOpt.has_value()) {
            callback({false, "User not found or system error", "", Json::Value()});
            return;
        }

        const auto& user = userOpt.value();
        std::string inputHash = Utils::hashPassword(password);

        // 校验密码
        if (inputHash != user.passwordHash) {
            callback({false, "Invalid password", "", Json::Value()});
            return;
        }

        // 生成 Token
        std::string token = Utils::generateUUID();
        // 30天过期
        long ttl = 30 * 24 * 3600; 

        // 保存会话
        sessionRepo->saveSession(user.id, token, ttl, [callback, token, user](bool success) {
            if (success) {
                LoginResult res;
                res.success = true;
                res.message = "Login successful";
                res.token = token;
                res.userData = user.toJson();
                callback(res);
            } else {
                callback({false, "Failed to create session", "", Json::Value()});
            }
        });
    });
}

void UserService::logout(const std::string& token, LogoutCallback callback) {
    sessionRepo->removeSession(token, callback);
}

void UserService::uploadAvatar(const std::string& userId, const std::string& localFilePath, const std::string& fileExt, std::function<void(bool, const std::string&)> callback) 
{
    auto& cfg = ConfigManager::instance();
    std::string cdnBase = "http://" + cfg.getCdnHost() + ":" + std::to_string(cfg.getCdnPort());

    // 上传到 MinIO
    // 命名规则: avatars/userid.ext
    std::string objectKey = "avatars/" + userId + fileExt;
    
    std::string avatarUrl = storageClient->uploadFile("public", objectKey, localFilePath, cdnBase);

    if (avatarUrl.empty()) {
        callback(false, "");
        return;
    }

    // 更新数据库
    userRepo->updateAvatar(userId, avatarUrl, [callback, avatarUrl](bool dbSuccess) {
        if (dbSuccess) {
            callback(true, avatarUrl);
        } else {
            callback(false, "");
        }
    });
}

void UserService::followUser(const std::string& currentUserId, const std::string& targetUserId, bool action, std::function<void(bool, const std::string&)> callback) 
{
    // 不能关注自己
    if (currentUserId == targetUserId) {
        callback(false, "Cannot follow yourself");
        return;
    }

    userRepo->updateFollowStatus(currentUserId, targetUserId, action, [this, currentUserId, targetUserId, action, callback](bool success, const std::string& msg) {
        if (success) {
            auto redis = drogon::app().getRedisClient();

            // 更新 Redis 缓存 (加速读取)
            std::string key = "user:following:" + currentUserId;
            redis->execCommandAsync(
                [](const drogon::nosql::RedisResult&){},
                [](const std::exception&){},
                action ? "SADD %s %s" : "SREM %s %s", 
                key.c_str(), targetUserId.c_str()
            );

            // 缓存失效 (删除两个人的 Profile 缓存)
            redis->execCommandAsync(
                [](const drogon::nosql::RedisResult&){}, [](const std::exception&){},
                "DEL user:profile:%s user:profile:%s", 
                currentUserId.c_str(), targetUserId.c_str()
            );

            callback(true, action ? "Followed" : "Unfollowed");
        } else {
            callback(false, msg);
        }
    });
}

void UserService::getUserProfile(const std::string& targetUserId, const std::string& currentUserId, std::function<void(const std::optional<lepai::entity::User>&, const std::string&)> callback) 
{
    auto redis = drogon::app().getRedisClient();
    std::string cacheKey = "user:profile:" + targetUserId;

    // 公共逻辑：检查关注并返回
    auto checkFollowAndReturn = [this, currentUserId, targetUserId, callback](lepai::entity::User user) {
        if (!currentUserId.empty() && currentUserId != targetUserId) {
            // 如果登录且看的不是自己，去查关注关系
            userRepo->checkIsFollowing(currentUserId, targetUserId, [callback, user](bool isFollowing) mutable {
                user.isFollowed = isFollowing;
                callback(user, "");
            });
        } else {
            // 看自己，或者游客
            callback(user, "");
        }
    };

    // 公共逻辑：DB 查询失败处理
    auto onDbError = [callback](const std::string& err) {
        callback(std::nullopt, err.empty() ? "User not found" : err);
    };

    // 查 Redis
    redis->execCommandAsync(
        [this, redis, cacheKey, targetUserId, checkFollowAndReturn, onDbError](const drogon::nosql::RedisResult& r) {
            if (r.type() == drogon::nosql::RedisResultType::kString) {
                // [Hit] 缓存命中
                std::string jsonStr = r.asString();
                Json::Value root;
                Json::Reader reader;
                if (!jsonStr.empty() && reader.parse(jsonStr, root)) {
                    lepai::entity::User user;
                    user.id = root["id"].asString();
                    user.username = root["username"].asString();
                    user.avatarUrl = root["avatar_url"].asString();
                    user.followingCount = root["following_count"].asInt();
                    user.followerCount = root["follower_count"].asInt();
                    
                    checkFollowAndReturn(user);
                    return;
                }
            }

            // [Miss] 缓存未命中，查 DB
            userRepo->findById(targetUserId, [this, redis, cacheKey, checkFollowAndReturn, onDbError](const std::optional<lepai::entity::User>& userOpt, const std::string& err) {
                if (!userOpt.has_value()) {
                    onDbError(err);
                    return;
                }

                auto user = userOpt.value();
                
                // 回写缓存
                Json::Value v = user.toJson(); 
                v.removeMember("is_followed"); // 清理个性化字段
                Json::FastWriter writer;
                
                redis->execCommandAsync(
                    [](const drogon::nosql::RedisResult&){}, [](const std::exception&){},
                    "SETEX %s 60 %s", cacheKey.c_str(), writer.write(v).c_str()
                );

                checkFollowAndReturn(user);
            });
        },
        [this, targetUserId, checkFollowAndReturn, onDbError](const std::exception& e) {
            // Redis 挂了，降级走 DB
            LOG_ERROR << "Redis Profile Error: " << e.what();

            userRepo->findById(targetUserId, [checkFollowAndReturn, onDbError](const std::optional<lepai::entity::User>& userOpt, const std::string& err) {
                if (!userOpt.has_value()) {
                    onDbError(err);
                    return;
                }
                // 返回结果
                checkFollowAndReturn(userOpt.value());
            });
        },
        "GET %s", cacheKey.c_str()
    );
}

void UserService::changeUsername(const std::string& userId, const std::string& newName, std::function<void(bool, const std::string&)> callback) 
{
    // 基础校验
    if (newName.empty() || newName.length() > 50) {
        callback(false, "Invalid username length");
        return;
    }

    userRepo->updateUsername(userId, newName, [callback, userId](bool success, const std::string& msg) {
        if (success) {
            // 缓存失效
            auto redis = drogon::app().getRedisClient();
            redis->execCommandAsync(
                [](const drogon::nosql::RedisResult&){}, 
                [](const std::exception&){},
                "DEL user:profile:%s", 
                userId.c_str()
            );

            callback(true, "Username updated");
        } else {
            callback(false, msg);
        }
    });
}

}
}