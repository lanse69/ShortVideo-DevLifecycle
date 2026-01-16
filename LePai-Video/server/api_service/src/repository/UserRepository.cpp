#include "UserRepository.h"

#include <drogon/drogon.h>
#include <sstream>

namespace lepai {
namespace repository {

void UserRepository::findByUsername(const std::string& username, DbResultCallback callback) 
{
    auto db = drogon::app().getDbClient("slave"); // 读操作走从库
    if (!db) {
        LOG_ERROR << "CRITICAL: DB Client 'slave' not found!";
        callback(std::nullopt, "Database connection error (slave)");
        return;
    }

    std::string sql = R"(
        SELECT 
            id, username, password_hash, created_at,
            avatar_url, following_count, follower_count 
        FROM users 
        WHERE username = $1
    )";

    db->execSqlAsync(
        sql,
        [callback](const drogon::orm::Result& r) {
            if (r.size() == 0) {
                callback(std::nullopt, "");
                return;
            }
            lepai::entity::User user;
            try {
                user.id = r[0]["id"].as<std::string>();
                user.username = r[0]["username"].as<std::string>();
                user.passwordHash = r[0]["password_hash"].as<std::string>();
                user.createdAt = r[0]["created_at"].as<std::string>();
                user.avatarUrl = r[0]["avatar_url"].isNull() ? "" : r[0]["avatar_url"].as<std::string>();
                user.followingCount = r[0]["following_count"].as<int>();
                user.followerCount = r[0]["follower_count"].as<int>();
                callback(user, "");
            } catch (const std::exception& e) {
                LOG_ERROR << "Data parsing error: " << e.what();
                callback(std::nullopt, "Data corruption");
            }
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            callback(std::nullopt, e.base().what());
        },
        username
    );
}

void UserRepository::createUser(const lepai::entity::User& user, CreateCallback callback) 
{
    auto db = drogon::app().getDbClient("default"); // 写操作走主库

    db->execSqlAsync(
        "INSERT INTO users (id, username, password_hash) VALUES ($1, $2, $3)",
        [callback](const drogon::orm::Result& r) {
            callback(true, "");
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            callback(false, e.base().what());
        },
        user.id, user.username, user.passwordHash
    );
}

void UserRepository::updateAvatar(const std::string& userId, const std::string& url, std::function<void(bool)> callback) 
{
    auto db = drogon::app().getDbClient("default");
    db->execSqlAsync(
        "UPDATE users SET avatar_url = $1 WHERE id = $2",
        [callback](const drogon::orm::Result&){ callback(true); },
        [callback](const drogon::orm::DrogonDbException& e){ 
            LOG_ERROR << "Update avatar failed: " << e.base().what();
            callback(false); 
        },
        url, userId
    );
}

void UserRepository::updateFollowStatus(const std::string& followerId, const std::string& followingId, bool isFollow, std::function<void(bool, const std::string&)> callback) 
{
    auto client = drogon::app().getDbClient("default"); // 写主库

    client->newTransactionAsync([followerId, followingId, isFollow, callback](const std::shared_ptr<drogon::orm::Transaction> &trans) {
        std::string sqlRel;
        if (isFollow) {
            sqlRel = "INSERT INTO user_follows (follower_id, following_id, created_at) VALUES ($1, $2, NOW()) ON CONFLICT DO NOTHING";
        } else {
            sqlRel = "DELETE FROM user_follows WHERE follower_id = $1 AND following_id = $2";
        }

        trans->execSqlAsync(
            sqlRel,
            [trans, followerId, followingId, isFollow, callback](const drogon::orm::Result &r) {
                if (r.affectedRows() == 0) {
                    callback(true, ""); 
                    return;
                }

                // 更新 follower (我) 的关注数
                std::string op = isFollow ? "+" : "-";
                std::string sqlUpdateA = "UPDATE users SET following_count = following_count " + op + " 1 WHERE id = $1";
                
                trans->execSqlAsync(
                    sqlUpdateA,
                    [trans, followingId, isFollow, callback](const drogon::orm::Result &rA) {
                        
                        // 更新 following (对方) 的粉丝数
                        std::string op = isFollow ? "+" : "-";
                        std::string sqlUpdateB = "UPDATE users SET follower_count = follower_count " + op + " 1 WHERE id = $1";

                        trans->execSqlAsync(
                            sqlUpdateB,
                            [trans, callback](const drogon::orm::Result &rB) {
                                callback(true, "");
                            },
                            [callback](const drogon::orm::DrogonDbException &e) {
                                LOG_ERROR << "Transaction Failed: " << e.base().what();
                                callback(false, "Failed to update follower count");
                            },
                            followingId
                        );
                    },
                    [callback](const drogon::orm::DrogonDbException &e) {
                        LOG_ERROR << "Transaction Failed: " << e.base().what();
                        callback(false, "Failed to update following count");
                    },
                    followerId
                );
            },
            [callback](const drogon::orm::DrogonDbException &e) {
                LOG_ERROR << "Transaction Failed: " << e.base().what();
                callback(false, "Database error");
            },
            followerId, followingId
        );
    });
}

void UserRepository::checkIsFollowing(const std::string& followerId, const std::string& followingId, std::function<void(bool)> callback) 
{
    auto client = drogon::app().getDbClient("slave"); // 读从库
    client->execSqlAsync(
        "SELECT 1 FROM user_follows WHERE follower_id = $1 AND following_id = $2 LIMIT 1",
        [callback](const drogon::orm::Result& r) {
            callback(r.size() > 0);
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "Check Follow DB Error: " << e.base().what();
            callback(false); // 降级处理：出错默认未关注
        },
        followerId, followingId
    );
}

void UserRepository::getFollowingIds(const std::string& followerId, const std::vector<std::string>& targetIds, std::function<void(const std::vector<std::string>&)> callback)
{
    if (targetIds.empty()) {
        callback({});
        return;
    }

    auto client = drogon::app().getDbClient("slave"); // 读从库

    std::string idListStr;
    for (size_t i = 0; i < targetIds.size(); ++i) {
        idListStr += targetIds[i];
        if (i < targetIds.size() - 1) {
            idListStr += ",";
        }
    }

    std::string sql = "SELECT following_id FROM user_follows WHERE follower_id = $1 AND following_id = ANY(string_to_array($2, ','))";

    client->execSqlAsync(
        sql,
        [callback](const drogon::orm::Result& r) {
            std::vector<std::string> followingIds;
            followingIds.reserve(r.size());
            for (const auto& row : r) {
                followingIds.push_back(row["following_id"].as<std::string>());
            }
            callback(followingIds);
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "Batch Follow Check Error: " << e.base().what();
            // 出错降级：返回空列表
            callback({});
        },
        followerId, idListStr
    );
}

void UserRepository::findById(const std::string& userId, std::function<void(const std::optional<lepai::entity::User>&, const std::string&)> callback) 
{
    // 读从库
    auto client = drogon::app().getDbClient("slave");
    
    client->execSqlAsync(
        "SELECT id, username, avatar_url, following_count, follower_count, created_at FROM users WHERE id = $1",
        [callback](const drogon::orm::Result& r) {
            if (r.size() == 0) {
                callback(std::nullopt, ""); // 用户不存在
                return;
            }
            lepai::entity::User user;
            try {
                user.id = r[0]["id"].as<std::string>();
                user.username = r[0]["username"].as<std::string>();
                user.avatarUrl = r[0]["avatar_url"].isNull() ? "" : r[0]["avatar_url"].as<std::string>();
                
                user.followingCount = r[0]["following_count"].as<int>();
                user.followerCount = r[0]["follower_count"].as<int>();
                
                user.createdAt = r[0]["created_at"].as<std::string>();
                callback(user, "");
            } catch (const std::exception& e) {
                LOG_ERROR << "Parse User Error: " << e.what();
                callback(std::nullopt, "Data corruption");
            }
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "Find User By ID Error: " << e.base().what();
            callback(std::nullopt, e.base().what());
        },
        userId
    );
}

void UserRepository::updateUsername(const std::string& userId, const std::string& newName, std::function<void(bool, const std::string&)> callback) 
{
    auto client = drogon::app().getDbClient("default"); // 写主库

    client->execSqlAsync(
        "UPDATE users SET username = $1 WHERE id = $2",
        [callback](const drogon::orm::Result& r) {
            if (r.affectedRows() > 0) {
                callback(true, "");
            } else {
                callback(false, "User not found");
            }
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            std::string err = e.base().what();
            LOG_ERROR << "Update Username Error: " << err;
            
            if (err.find("unique constraint") != std::string::npos || 
                err.find("duplicate key") != std::string::npos) {
                callback(false, "Username already exists");
            } else {
                callback(false, "Database error");
            }
        },
        newName, userId
    );
}

}
}