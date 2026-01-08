#pragma once

#include <string>
#include <json/json.h>

namespace lepai {
namespace domain {

struct User {
    std::string id;
    std::string username;
    std::string passwordHash;
    std::string createdAt;

    Json::Value toJson() const {
        Json::Value v;
        v["id"] = id;
        v["username"] = username;
        // 密码哈希不返回给前端
        v["createdAt"] = createdAt;
        return v;
    }
};

} // namespace domain
} // namespace lepai