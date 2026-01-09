#include "UserService.h"

#include <drogon/drogon.h>

#include "utils.h"

namespace lepai {
namespace service {

UserService::UserService() {
    userRepo = std::make_shared<lepai::repository::UserRepository>();
    sessionRepo = std::make_shared<lepai::repository::SessionRepository>();
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

}
}