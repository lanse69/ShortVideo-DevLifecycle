#pragma once

#include <string>
#include <json/json.h>

class ConfigManager {
public:
    static ConfigManager& instance();
    bool loadConfig(const std::string& configPath);

    // 主库 (Write)
    std::string getDbMasterHost() const;
    int getDbMasterPort() const;

    // 从库 (Read)
    std::string getDbSlaveHost() const;
    int getDbSlavePort() const;

    std::string getRedisHost() const;
    std::string getMinioHost() const;

private:
    ConfigManager() = default;
    Json::Value m_config;
};