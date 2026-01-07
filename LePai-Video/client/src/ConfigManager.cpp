#include "ConfigManager.h"

#include <QFile>
#include <QJsonDocument>
#include <QDebug>

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::loadConfig(const QString& configPath) {
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[Client] 无法打开配置文件:" << configPath;
        return false;
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "[Client] 配置文件格式错误 (非 JSON)";
        return false;
    }
    
    m_config = doc.object();
    qInfo() << "[Client] 配置加载成功:" << m_config;
    return true;
}

QString ConfigManager::getApiServerUrl() const {
    return m_config["api_server_url"].toString("http://127.0.0.1:8080");
}

QString ConfigManager::getCdnServerUrl() const {
    return m_config["cdn_server_url"].toString("http://127.0.0.1:80");
}

QString ConfigManager::getUploadEndpoint() const {
    return m_config["upload_endpoint"].toString("http://127.0.0.1:9000");
}