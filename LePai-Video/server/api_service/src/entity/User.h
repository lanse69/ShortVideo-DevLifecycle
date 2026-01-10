#pragma once

#include <string>
#include <json/json.h>

namespace lepai {
namespace entity {

struct User {
    std::string id;
    std::string username;
    std::string passwordHash;
    std::string avatarUrl;
    std::string createdAt;
    
    int followingCount = 0;
    int followerCount = 0;

    bool isFollowed = false; // 当前登录用户是否关注了该用户

    Json::Value toJson() const {
        Json::Value v;
        v["id"] = id;
        v["username"] = username;
        v["avatar_url"] = avatarUrl;
        // 密码哈希不返回
        v["following_count"] = followingCount;
        v["follower_count"] = followerCount;
        v["is_followed"] = isFollowed;
        return v;
    }
};

} // namespace entity
} // namespace lepai