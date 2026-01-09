#pragma once

#include <string>

namespace Utils {
    /**
     * 生成 UUID
     */
    std::string generateUUID();

    /**
     * 使用 SHA256 对密码进行哈希
     */
    std::string hashPassword(const std::string& plainPassword);
}