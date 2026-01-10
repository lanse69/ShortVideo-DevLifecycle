#pragma once

#include <string>
#include <functional>
#include <json/json.h>
#include <memory>

#include "../repository/UserRepository.h"
#include "../repository/SessionRepository.h"
#include "MinioClient.h"

namespace lepai {
namespace service {

struct LoginResult {
    bool success;
    std::string message;
    std::string token;
    Json::Value userData;
};

class UserService {
public:
    using RegisterCallback = std::function<void(bool success, const std::string& message)>;
    using LoginCallback = std::function<void(const LoginResult& result)>;
    using LogoutCallback = std::function<void(bool success)>;

    UserService();

    // 注册业务逻辑
    void registerUser(const std::string& username, const std::string& password, RegisterCallback callback);

    // 登录业务逻辑
    void login(const std::string& username, const std::string& password, LoginCallback callback);
    
    // 登出业务逻辑
    void logout(const std::string& token, LogoutCallback callback);

    // 上传头像业务
    void uploadAvatar(const std::string& userId, const std::string& localFilePath, const std::string& fileExt, std::function<void(bool success, const std::string& url)> callback);

private:
    std::shared_ptr<lepai::repository::UserRepository> userRepo;
    std::shared_ptr<lepai::repository::SessionRepository> sessionRepo;
    std::unique_ptr<lepai::storage::MinioClient> storageClient;
};

}
}