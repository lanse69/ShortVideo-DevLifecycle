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

    // 更新用户头像
    void updateAvatar(const std::string& userId, const std::string& url, std::function<void(bool)> callback);

    // 关注/取关
    // isFollow: true=关注, false=取关
    void updateFollowStatus(const std::string& followerId, const std::string& followingId, bool isFollow, std::function<void(bool success, const std::string& err)> callback);

    // 检查 A 是否关注了 B
    void checkIsFollowing(const std::string& followerId, const std::string& followingId, std::function<void(bool isFollowing)> callback);

    // 批量检查 A 是否关注了列表中的某些人
    // 返回被 A 关注的 ID 列表
    void getFollowingIds(const std::string& followerId, const std::vector<std::string>& targetIds, std::function<void(const std::vector<std::string>&)> callback);

    // 根据 ID 获取用户详情
    void findById(const std::string& userId, std::function<void(const std::optional<lepai::entity::User>&, const std::string& err)> callback);

    // 修改用户名
    void updateUsername(const std::string& userId, const std::string& newName, std::function<void(bool success, const std::string& err)> callback);
};

}
}