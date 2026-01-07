#pragma once

#include <drogon/HttpController.h>

class UserController : public drogon::HttpController<UserController>
{
public:
    METHOD_LIST_BEGIN
    // 注册路由: POST /api/user/register
    ADD_METHOD_TO(UserController::registerUser, "/api/user/register", drogon::Post);
    METHOD_LIST_END

    void registerUser(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback);
};