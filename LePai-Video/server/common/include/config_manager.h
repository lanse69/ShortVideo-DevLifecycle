#pragma once

#include <QString>
#include <QJsonObject>

class ConfigManager {
public:
    static ConfigManager& instance();
    bool loadConfig(const QString& configPath);

    QString getDbHost() const;
    int getDbPort() const;
    QString getRedisHost() const;
    QString getMinioHost() const;

private:
    ConfigManager() = default;
    QJsonObject m_config;
};
