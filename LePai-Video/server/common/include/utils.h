#pragma once

#include <QString>

namespace Utils {
    /**
     * 生成 UUID
     */
    QString generateUUID();

    /**
     * 使用 SHA256 对密码进行哈希
     */
    QString hashPassword(const QString& plainPassword);
}
