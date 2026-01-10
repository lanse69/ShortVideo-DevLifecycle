#include "UserController.h"

#include <filesystem>

#include "config_manager.h"

UserController::UserController() {
    userService = std::make_shared<lepai::service::UserService>();
}

void UserController::registerUser(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody("Invalid JSON");
        callback(resp);
        return;
    }

    std::string username;
    std::string password;
    if (jsonPtr->isMember("username")) username = (*jsonPtr)["username"].asString();
    if (jsonPtr->isMember("password")) password = (*jsonPtr)["password"].asString();

    if (username.empty() || password.empty()) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody("Username and password required");
        callback(resp);
        return;
    }

    // 调用应用服务层
    userService->registerUser(username, password, [callback](bool success, const std::string& msg) {
        Json::Value ret;
        ret["code"] = success ? 200 : 400;
        ret["message"] = msg;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
        if(!success) resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
    });
}

void UserController::loginUser(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody("Invalid JSON");
        callback(resp);
        return;
    }

    std::string username;
    std::string password;
    if (jsonPtr->isMember("username")) username = (*jsonPtr)["username"].asString();
    if (jsonPtr->isMember("password")) password = (*jsonPtr)["password"].asString();

    userService->login(username, password, [callback](const lepai::service::LoginResult& result) {
        Json::Value ret;
        if (result.success) {
            ret["code"] = 200;
            ret["message"] = result.message;
            ret["token"] = result.token;
            ret["user"] = result.userData;
        } else {
            ret["code"] = 401;
            ret["message"] = result.message;
        }
        
        auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
        if (!result.success) resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
    });
}

void UserController::logoutUser(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    // 从 Header 获取 Token: "Authorization: Bearer <token>"
    std::string token = req->getHeader("Authorization");
    if (token.size() > 7 && token.substr(0, 7) == "Bearer ") {
        token = token.substr(7);
    }

    if (token.empty()) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k401Unauthorized);
        resp->setBody("No token provided");
        callback(resp);
        return;
    }

    userService->logout(token, [callback](bool success) {
        Json::Value ret;
        ret["code"] = 200;
        ret["message"] = "Logged out";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
        callback(resp);
    });
}

void UserController::updateAvatar(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    // 直接从 Filter 注入的属性中获取 userId
    std::string userId = req->attributes()->get<std::string>("userId");
    
    if (userId.empty()) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k401Unauthorized); // 双重保险
        callback(resp);
        return;
    }

    // 处理文件上传
    drogon::MultiPartParser parser;
    if (parser.parse(req) != 0 || parser.getFiles().empty()) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody("No file uploaded");
        callback(resp);
        return;
    }

    auto& file = parser.getFiles()[0];
    std::string ext = std::filesystem::path(file.getFileName()).extension().string();
    if(ext.empty()) ext = ".jpg";

    // 保存临时文件
    std::string tempPath = "/tmp/" + userId + "_avatar" + ext;
    file.saveAs(tempPath);

    userService->uploadAvatar(userId, tempPath, ext, [callback, tempPath](bool success, const std::string& url) {
        // 清理临时文件
        std::filesystem::remove(tempPath);

        Json::Value ret;
        if (success) {
            ret["code"] = 200;
            ret["message"] = "Avatar updated";
            ret["avatar_url"] = url;
        } else {
            ret["code"] = 500;
            ret["message"] = "Upload failed";
        }
        callback(drogon::HttpResponse::newHttpJsonResponse(ret));
    });
}