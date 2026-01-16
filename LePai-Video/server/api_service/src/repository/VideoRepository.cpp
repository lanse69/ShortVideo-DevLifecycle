#include "VideoRepository.h"

#include <sstream>
#include <algorithm>

namespace lepai {
namespace repository {

void VideoRepository::getGlobalFeed(long long limit, long long offset, FeedCallback callback) 
{
    // 走从库
    auto db = drogon::app().getDbClient("slave");
    if (!db) {
        callback({}, "DB slave unavailable");
        return;
    }

    // 联表查询：视频表 + 用户表
    std::string sql = R"(
        SELECT 
            v.id, v.user_id, v.title, v.url, v.cover_url, v.duration, v.like_count, v.created_at,
            u.username, u.avatar_url
        FROM videos v
        LEFT JOIN users u ON v.user_id = u.id
        WHERE v.status = 1 
        ORDER BY v.created_at DESC 
        LIMIT $1 OFFSET $2
    )";

    db->execSqlAsync(
        sql,
        [callback](const drogon::orm::Result& r) {
            std::vector<lepai::entity::Video> videos;
            videos.reserve(r.size());
            
            for (const auto& row : r) {
                lepai::entity::Video v;
                try {
                    v.id = row["id"].as<std::string>();
                    v.userId = row["user_id"].isNull() ? "" : row["user_id"].as<std::string>();
                    v.title = row["title"].as<std::string>();
                    v.url = row["url"].as<std::string>();
                    v.coverUrl = row["cover_url"].isNull() ? "" : row["cover_url"].as<std::string>();
                    v.duration = row["duration"].as<int>();
                    v.likeCount = row["like_count"].as<long long>();
                    v.createdAt = row["created_at"].as<std::string>();
                    v.authorName = row["username"].isNull() ? "Unknown" : row["username"].as<std::string>();
                    v.authorAvatar = row["avatar_url"].isNull() ? "" : row["avatar_url"].as<std::string>();
                    videos.emplace_back(std::move(v));
                } catch (const std::exception& e) {
                    LOG_ERROR << "Error parsing video row: " << e.what();
                }
            }
            callback(videos, "");
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "Feed Query Error: " << e.base().what();
            callback({}, "Internal DB Error");
        },
        limit, offset
    );
}

void VideoRepository::getLikedVideoIds(const std::string& userId, const std::vector<std::string>& videoIds, LikeStatusCallback callback) 
{
    if (videoIds.empty()) {
        callback({}, "");
        return;
    }

    auto db = drogon::app().getDbClient("slave");
    if (!db) {
        callback({}, "DB slave unavailable");
        return;
    }

    std::string idListStr;
    for (size_t i = 0; i < videoIds.size(); ++i) {
        idListStr += videoIds[i];
        if (i < videoIds.size() - 1) {
            idListStr += ",";
        }
    }

    // 查询这些视频中，哪些被该用户点赞过
    std::string sql = "SELECT video_id FROM video_likes WHERE user_id = $1 AND video_id = ANY(string_to_array($2, ','))";

    db->execSqlAsync(
        sql,
        [callback](const drogon::orm::Result& r) {
            std::vector<std::string> likedIds;
            for (const auto& row : r) {
                likedIds.push_back(row["video_id"].as<std::string>());
            }
            callback(likedIds, "");
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "Like Check Error: " << e.base().what();
            callback({}, "DB Error checking likes");
        },
        userId, idListStr
    );
}

void VideoRepository::createVideo(const lepai::entity::Video& video, VoidCallback callback) 
{
    auto db = drogon::app().getDbClient("default"); // 写主库
    
    db->execSqlAsync(
        "INSERT INTO videos (id, user_id, title, url, status, created_at) VALUES ($1, $2, $3, $4, 0, NOW())",
        [callback](const drogon::orm::Result& r) {
            callback(true, "");
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "Create Video DB Error: " << e.base().what();
            callback(false, e.base().what());
        },
        video.id, video.userId, video.title, video.url
    );
}

// 添加点赞
void VideoRepository::addLikeRecord(const std::string& userId, const std::string& videoId, VoidCallback callback) 
{
    auto db = drogon::app().getDbClient("default"); // 写主库
    
    db->execSqlAsync(
        "INSERT INTO video_likes (user_id, video_id, created_at) VALUES ($1, $2, NOW()) ON CONFLICT DO NOTHING",
        [callback](const drogon::orm::Result& r) {
            if (r.affectedRows() > 0) {
                callback(true, "");
            } else {
                callback(false, "Already liked");
            }
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "Add Like DB Error: " << e.base().what();
            callback(false, e.base().what());
        },
        userId, videoId
    );
}

// 取消点赞
void VideoRepository::removeLikeRecord(const std::string& userId, const std::string& videoId, VoidCallback callback) 
{
    auto db = drogon::app().getDbClient("default");
    
    db->execSqlAsync(
        "DELETE FROM video_likes WHERE user_id = $1 AND video_id = $2",
        [callback](const drogon::orm::Result& r) {
            if (r.affectedRows() > 0) {
                callback(true, "");
            } else {
                callback(false, "Like record not found");
            }
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "Remove Like DB Error: " << e.base().what();
            callback(false, e.base().what());
        },
        userId, videoId
    );
}

void VideoRepository::getVideoLikeCount(const std::string& videoId, std::function<void(long long)> callback)
{
    // 读从库
    auto db = drogon::app().getDbClient("slave");
    db->execSqlAsync(
        "SELECT like_count FROM videos WHERE id = $1",
        [callback](const drogon::orm::Result& r) {
            if (r.size() > 0) {
                callback(r[0]["like_count"].as<long long>());
            } else {
                callback(0);
            }
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "Get Like Count DB Error: " << e.base().what();
            callback(0);
        },
        videoId
    );
}

// 关注视频流
void VideoRepository::getFollowingFeed(const std::string& userId, long long limit, long long offset, FeedCallback callback) 
{
    auto db = drogon::app().getDbClient("slave");
    if (!db) {
        callback({}, "DB slave unavailable");
        return;
    }
    
    // 联表查询：视频表 + 关注表 + 用户表
    std::string sql = R"(
        SELECT 
            v.id, v.user_id, v.title, v.url, v.cover_url, v.duration, v.like_count, v.created_at,
            u.username, u.avatar_url
        FROM videos v
        JOIN user_follows f ON v.user_id = f.following_id
        LEFT JOIN users u ON v.user_id = u.id
        WHERE f.follower_id = $1 AND v.status = 1
        ORDER BY v.created_at DESC
        LIMIT $2 OFFSET $3
    )";

    db->execSqlAsync(
        sql,
        [callback](const drogon::orm::Result& r) {
            std::vector<lepai::entity::Video> videos;
            videos.reserve(r.size());

            for (const auto& row : r) {
                lepai::entity::Video v;
                try {
                    v.id = row["id"].as<std::string>();
                    v.userId = row["user_id"].as<std::string>();
                    v.title = row["title"].as<std::string>();
                    v.url = row["url"].as<std::string>();
                    v.coverUrl = row["cover_url"].isNull() ? "" : row["cover_url"].as<std::string>();
                    v.duration = row["duration"].as<int>();
                    v.likeCount = row["like_count"].as<long long>();
                    v.createdAt = row["created_at"].as<std::string>();
                    v.authorName = row["username"].isNull() ? "Unknown" : row["username"].as<std::string>();
                    v.authorAvatar = row["avatar_url"].isNull() ? "" : row["avatar_url"].as<std::string>();
                    videos.emplace_back(std::move(v));
                } catch (const std::exception& e) {
                    LOG_ERROR << "Error parsing video row: " << e.what();
                }
            }
            callback(videos, "");
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "Following Feed Query Error: " << e.base().what();
            callback({}, "Internal DB Error");
        },
        userId, limit, offset
    );
}

// 点赞流查询
void VideoRepository::getLikedFeed(const std::string& targetUserId, long long limit, long long offset, FeedCallback callback) 
{
    auto db = drogon::app().getDbClient("slave");
    if (!db) {
        callback({}, "DB slave unavailable");
        return;
    }

    // 联表查询：视频表 + 点赞表 + 用户表
    std::string sql = R"(
        SELECT 
            v.id, v.user_id, v.title, v.url, v.cover_url, v.duration, v.like_count, v.created_at,
            u.username, u.avatar_url
        FROM video_likes vl
        JOIN videos v ON vl.video_id = v.id
        LEFT JOIN users u ON v.user_id = u.id
        WHERE vl.user_id = $1 AND v.status = 1
        ORDER BY vl.created_at DESC
        LIMIT $2 OFFSET $3
    )";

    db->execSqlAsync(
        sql,
        [callback](const drogon::orm::Result& r) {
            std::vector<lepai::entity::Video> videos;
            videos.reserve(r.size());

            for (const auto& row : r) {
                lepai::entity::Video v;
                try {
                    v.id = row["id"].as<std::string>();
                    v.userId = row["user_id"].as<std::string>();
                    v.title = row["title"].as<std::string>();
                    v.url = row["url"].as<std::string>();
                    v.coverUrl = row["cover_url"].isNull() ? "" : row["cover_url"].as<std::string>();
                    v.duration = row["duration"].as<int>();
                    v.likeCount = row["like_count"].as<long long>();
                    v.createdAt = row["created_at"].as<std::string>();
                    v.authorName = row["username"].isNull() ? "Unknown" : row["username"].as<std::string>();
                    v.authorAvatar = row["avatar_url"].isNull() ? "" : row["avatar_url"].as<std::string>();
                    videos.emplace_back(std::move(v));
                } catch (const std::exception& e) {
                    LOG_ERROR << "Error parsing liked video row: " << e.what();
                }
            }
            callback(videos, "");
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "Liked Feed Query Error: " << e.base().what();
            callback({}, "Internal DB Error");
        },
        targetUserId, limit, offset
    );
}

// 用户发布列表查询
void VideoRepository::getUserUploadFeed(const std::string& targetUserId, long long limit, long long offset, FeedCallback callback) 
{
    auto db = drogon::app().getDbClient("slave");
    if (!db) {
        callback({}, "DB slave unavailable");
        return;
    }

    // 联表查询：视频表 + 用户表
    std::string sql = R"(
        SELECT 
            v.id, v.user_id, v.title, v.url, v.cover_url, v.duration, v.like_count, v.created_at,
            u.username, u.avatar_url
        FROM videos v
        LEFT JOIN users u ON v.user_id = u.id
        WHERE v.user_id = $1 AND v.status = 1
        ORDER BY v.created_at DESC
        LIMIT $2 OFFSET $3
    )";

    db->execSqlAsync(
        sql,
        [callback](const drogon::orm::Result& r) {
            std::vector<lepai::entity::Video> videos;
            videos.reserve(r.size());

            for (const auto& row : r) {
                lepai::entity::Video v;
                try {
                    v.id = row["id"].as<std::string>();
                    v.userId = row["user_id"].as<std::string>();
                    v.title = row["title"].as<std::string>();
                    v.url = row["url"].as<std::string>();
                    v.coverUrl = row["cover_url"].isNull() ? "" : row["cover_url"].as<std::string>();
                    v.duration = row["duration"].as<int>();
                    v.likeCount = row["like_count"].as<long long>();
                    v.createdAt = row["created_at"].as<std::string>();
                    v.authorName = row["username"].isNull() ? "Unknown" : row["username"].as<std::string>();
                    v.authorAvatar = row["avatar_url"].isNull() ? "" : row["avatar_url"].as<std::string>();
                    videos.emplace_back(std::move(v));
                } catch (const std::exception& e) {
                    LOG_ERROR << "Error parsing user upload video row: " << e.what();
                }
            }
            callback(videos, "");
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "User Upload Feed Query Error: " << e.base().what();
            callback({}, "Internal DB Error");
        },
        targetUserId, limit, offset
    );
}

}
}