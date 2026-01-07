#include "UserController.h"

#include <drogon/orm/DbClient.h>
#include <drogon/HttpAppFramework.h>

#include "utils.h"

void UserController::registerUser(const drogon::HttpRequestPtr& req, std::function<void (const drogon::HttpResponsePtr &)> &&callback)
{
    // 解析 JSON 请求体
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody("Invalid JSON format");
        callback(resp);
        return;
    }

    std::string username = (*jsonPtr)["username"].asString();
    std::string password = (*jsonPtr)["password"].asString();

    if (username.empty() || password.empty()) {
        auto resp = drogon::HttpResponse::newHttpResponse();
        resp->setStatusCode(drogon::k400BadRequest);
        resp->setBody("Username and password are required");
        callback(resp);
        return;
    }

    // 生成 ID 和 密码哈希
    QString uuid = Utils::generateUUID();
    QString pwdHash = Utils::hashPassword(QString::fromStdString(password));

    // 获取数据库客户端
    auto db = drogon::app().getDbClient("default");

    // 异步 SQL 插入
    db->execSqlAsync(
        "INSERT INTO users (id, username, password_hash) VALUES ($1, $2, $3)",
        [callback](const drogon::orm::Result &r) {
            // 成功
            Json::Value ret;
            ret["code"] = 200;
            ret["message"] = "Registration successful";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
            callback(resp);
        },
        [callback](const drogon::orm::DrogonDbException &e) {
            // 失败
            Json::Value ret;
            ret["code"] = 500;
            ret["message"] = "Registration failed";
            ret["details"] = e.base().what(); 
            
            auto resp = drogon::HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },
        uuid.toStdString(), 
        username, 
        pwdHash.toStdString()
    );
}