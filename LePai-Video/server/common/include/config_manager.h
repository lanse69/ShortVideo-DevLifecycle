#pragma once

#include <string>
#include <json/json.h>

class ConfigManager {
public:
    static ConfigManager& instance();
    bool loadConfig(const std::string& configPath);

    // 数据库
    std::string getDbMasterHost() const;
    int getDbMasterPort() const;
    std::string getDbSlaveHost() const;
    int getDbSlavePort() const;
    std::string getDbName() const;
    std::string getDbUser() const;
    std::string getDbPassword() const;

    // Redis
    std::string getRedisHost() const;
    int getRedisPort() const;
    std::string getRedisPassword() const;

    // MinIO
    std::string getMinioHost() const;
    int getMinioPort() const;
    std::string getMinioUser() const;
    std::string getMinioPassword() const;

    // CDN
    std::string getCdnHost() const;
    int getCdnPort() const;

private:
    ConfigManager() = default;
    Json::Value m_config;
};