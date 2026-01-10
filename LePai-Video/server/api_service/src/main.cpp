#include <drogon/drogon.h>
#include <filesystem>

#include "config_manager.h"
#include "scheduler/SyncScheduler.h"

int main(int argc, char *argv[]) {
    // 获取配置路径
    std::filesystem::path exePath = std::filesystem::canonical(std::filesystem::path("/proc/self/exe")).parent_path();
    std::string configPathStr = (exePath / "config.json").string();

    // 加载配置
    if (!ConfigManager::instance().loadConfig(configPathStr)) {
        LOG_ERROR << "Failed to load config from " << configPathStr;
        return 1;
    }

    auto& cfg = ConfigManager::instance();

    drogon::app().setLogLocalTime(true); 

    LOG_INFO << "Starting API Server...";

    // 主库
    LOG_INFO << "Master DB: " << cfg.getDbMasterHost() << ":" << cfg.getDbMasterPort();
    drogon::app().createDbClient("postgresql", 
        cfg.getDbMasterHost(),
        cfg.getDbMasterPort(),
        cfg.getDbName(),
        cfg.getDbUser(),
        cfg.getDbPassword(),
        1,
        "default",
        "default"
    );

    // 从库
    LOG_INFO << "Slave DB : " << cfg.getDbSlaveHost() << ":" << cfg.getDbSlavePort();
    drogon::app().createDbClient("postgresql", 
        cfg.getDbSlaveHost(),
        cfg.getDbSlavePort(),
        cfg.getDbName(),
        cfg.getDbUser(),
        cfg.getDbPassword(),
        10,
        "default",
        "slave"
    );

    // Redis
    LOG_INFO << "Redis Host: " << cfg.getRedisHost();
    drogon::app().createRedisClient(
        cfg.getRedisHost(),
        cfg.getRedisPort(),
        "default",
        cfg.getRedisPassword()
    );

    // 定时任务
    drogon::app().registerBeginningAdvice([]() {
        drogon::app().getLoop()->runEvery(10.0, []() {
            lepai::scheduler::SyncScheduler::syncLikesToDB();
        });
    });

    drogon::app().addListener("0.0.0.0", 8080);
    drogon::app().run();

    return 0;
}