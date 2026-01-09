#include "UserRepository.h"

#include <drogon/drogon.h>

namespace lepai {
namespace repository {

void UserRepository::findByUsername(const std::string& username, DbResultCallback callback) {
    auto db = drogon::app().getDbClient("slave"); // 读操作走从库
    if (!db) {
        LOG_ERROR << "CRITICAL: DB Client 'slave' not found!";
        callback(std::nullopt, "Database connection error (slave)");
        return;
    }

    db->execSqlAsync(
        "SELECT id, username, password_hash, created_at FROM users WHERE username = $1",
        [callback](const drogon::orm::Result& r) {
            if (r.size() == 0) {
                callback(std::nullopt, "");
                return;
            }
            lepai::domain::User user;
            try {
                user.id = r[0]["id"].as<std::string>();
                user.username = r[0]["username"].as<std::string>();
                user.passwordHash = r[0]["password_hash"].as<std::string>();
                user.createdAt = r[0]["created_at"].as<std::string>();
                callback(user, "");
            } catch (const std::exception& e) {
                LOG_ERROR << "Data parsing error:" << e.what();
                callback(std::nullopt, "Data corruption");
            }
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            callback(std::nullopt, e.base().what());
        },
        username
    );
}

void UserRepository::createUser(const lepai::domain::User& user, CreateCallback callback) {
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

}
}