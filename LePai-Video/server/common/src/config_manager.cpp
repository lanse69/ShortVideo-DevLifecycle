#include "config_manager.h"

#include <fstream>
#include <iostream>
#include <drogon/drogon.h>

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::loadConfig(const std::string& configPath) {
    std::ifstream configFile(configPath, std::ifstream::binary);
    if (!configFile.is_open()) {
        LOG_ERROR << "Failed to open config file: " << configPath;
        return false;
    }

    Json::CharReaderBuilder builder;
    std::string errs;
    if (!Json::parseFromStream(builder, configFile, &m_config, &errs)) {
        LOG_ERROR << "Config parse error: " << errs;
        return false;
    }
    return true;
}

// 主库配置
std::string ConfigManager::getDbMasterHost() const { 
    return m_config.get("db_master_host", "127.0.0.1").asString();
}

int ConfigManager::getDbMasterPort() const { 
    return m_config.get("db_master_port", 5432).asInt();
}

// 从库配置
std::string ConfigManager::getDbSlaveHost() const { 
    return m_config.get("db_slave_host", "127.0.0.1").asString();
}

int ConfigManager::getDbSlavePort() const { 
    return m_config.get("db_slave_port", 5433).asInt();
}

std::string ConfigManager::getRedisHost() const { 
    return m_config.get("redis_host", "127.0.0.1").asString();
}

std::string ConfigManager::getMinioHost() const { 
    return m_config.get("minio_host", "127.0.0.1").asString();
}

int ConfigManager::getMinioPort() const {
    return m_config.get("minio_port", 9000).asInt();
}

std::string ConfigManager::getCdnHost() const {
    return m_config.get("cdn_host", "127.0.0.1").asString();
}

int ConfigManager::getCdnPort() const {
    return m_config.get("cdn_port", 80).asInt();
}