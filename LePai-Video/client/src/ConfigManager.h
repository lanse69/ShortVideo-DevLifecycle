#pragma once

#include <QString>
#include <QJsonObject>
#include <QObject>

class ConfigManager : public QObject {
    Q_OBJECT
public:
    static ConfigManager& instance();

    // 加载配置文件
    bool loadConfig(const QString& configPath);

    // Getters
    QString getApiServerUrl() const;
    QString getCdnServerUrl() const;
    QString getUploadEndpoint() const;

private:
    ConfigManager() = default;
    QJsonObject m_config;
};