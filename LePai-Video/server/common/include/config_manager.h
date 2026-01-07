#pragma once

#include <QString>
#include <QJsonObject>

class ConfigManager {
public:
    static ConfigManager& instance();
    bool loadConfig(const QString& configPath);

    // 主库 (Write)
    QString getDbMasterHost() const;
    int getDbMasterPort() const;

    // 从库 (Read)
    QString getDbSlaveHost() const;
    int getDbSlavePort() const;

    QString getRedisHost() const;
    QString getMinioHost() const;

private:
    ConfigManager() = default;
    QJsonObject m_config;
};
