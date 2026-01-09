#pragma once

#include <drogon/drogon.h>
#include <functional>
#include <optional>

#include "../entity/User.h"

namespace lepai {
namespace repository {

class UserRepository {
public:
    using DbResultCallback = std::function<void(const std::optional<lepai::entity::User>&, const std::string& error)>;
    using CreateCallback = std::function<void(bool success, const std::string& error)>;

    // 根据用户名查找用户
    void findByUsername(const std::string& username, DbResultCallback callback);

    // 创建新用户
    void createUser(const lepai::entity::User& user, CreateCallback callback);
};

}
}