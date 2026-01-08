#pragma once

#include <drogon/drogon.h>
#include <functional>

namespace lepai {
namespace repository {

class SessionRepository {
public:
    using VoidCallback = std::function<void(bool success)>;

    // 保存会话并处理互斥登录
    void saveSession(const std::string& userId, const std::string& token, long timeoutSeconds, VoidCallback callback);
    
    // 移除会话（登出）
    void removeSession(const std::string& token, VoidCallback callback);
};
    
}
}