#include "config_manager.h"

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
        qWarning() << "无法打开配置文件:" << configPath;
        return false;
    }
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "配置文件格式错误 (非 JSON)";
        return false;
    }
    m_config = doc.object();
    return true;
}

// 主库配置 (默认 5432)
QString ConfigManager::getDbMasterHost() const { 
    return m_config["db_master_host"].toString("127.0.0.1"); 
}

int ConfigManager::getDbMasterPort() const { 
    return m_config["db_master_port"].toInt(5432); 
}

// 从库配置 (默认 5433)
QString ConfigManager::getDbSlaveHost() const { 
    return m_config["db_slave_host"].toString("127.0.0.1"); 
}

int ConfigManager::getDbSlavePort() const { 
    return m_config["db_slave_port"].toInt(5433); 
}

QString ConfigManager::getRedisHost() const { 
    return m_config["redis_host"].toString("127.0.0.1"); 
}

QString ConfigManager::getMinioHost() const { 
    return m_config["minio_host"].toString("127.0.0.1"); 
}
