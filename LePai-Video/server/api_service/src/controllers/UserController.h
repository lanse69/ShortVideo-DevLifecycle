#pragma once

#include <drogon/HttpController.h>
#include <memory>

#include "../service/UserService.h"

class UserController : public drogon::HttpController<UserController>
{
public:
    METHOD_LIST_BEGIN
    // POST /api/user/register
    ADD_METHOD_TO(UserController::registerUser, "/api/user/register", drogon::Post);
    // POST /api/user/login
    ADD_METHOD_TO(UserController::loginUser, "/api/user/login", drogon::Post);
    // POST /api/user/logout
    ADD_METHOD_TO(UserController::logoutUser, "/api/user/logout", drogon::Post, "LoginFilter");
    // 头像上传
    ADD_METHOD_TO(UserController::updateAvatar, "/api/user/avatar", drogon::Post, "LoginFilter"); 
    METHOD_LIST_END

    UserController();

    void registerUser(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
    void loginUser(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
    void logoutUser(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
    void updateAvatar(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);

private:
    std::shared_ptr<lepai::service::UserService> userService;
};