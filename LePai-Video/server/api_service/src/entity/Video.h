#pragma once

#include <string>
#include <vector>
#include <json/json.h>

namespace lepai {
namespace entity {

struct Video {
    std::string id;
    std::string userId;
    std::string title;
    std::string url;        // 视频 CDN 地址
    std::string coverUrl;   // 封面 CDN 地址
    int duration = 0;
    long long likeCount = 0;
    std::string createdAt;
    
    // 作者信息
    std::string authorName;
    std::string authorAvatar;

    // 个性化状态 (仅登录用户有效)
    bool isLiked = false;
    bool isFollowed = false;

    // 序列化为 JSON
    Json::Value toJson() const {
        Json::Value v;
        v["id"] = id;
        v["title"] = title;
        v["url"] = url;
        v["cover_url"] = coverUrl;
        v["duration"] = duration;
        v["like_count"] = (Json::Int64)likeCount;
        v["author_id"] = userId;
        v["author"] = authorName;
        v["author_avatar"] = authorAvatar;
        v["is_liked"] = isLiked;
        v["is_followed"] = isFollowed;
        return v;
    }
    
    // 反序列化
    static Video fromJson(const Json::Value& v) {
        Video video;
        video.id = v.get("id", "").asString();
        video.title = v.get("title", "").asString();
        video.url = v.get("url", "").asString();
        video.coverUrl = v.get("cover_url", "").asString();
        video.duration = v.get("duration", 0).asInt();
        video.likeCount = v.get("like_count", 0).asInt64();
        if (v.isMember("author_id")) {
            video.userId = v["author_id"].asString();
        } else if (v.isMember("user_id")) {
            video.userId = v["user_id"].asString();
        }
        video.authorName = v.get("author", "Unknown").asString();
        video.authorAvatar = v.get("author_avatar", "").asString();
        video.isLiked = false; // 缓存中不存个性化状态
        video.isFollowed = false;
        return video;
    }
};

}
}