#include <drogon/drogon.h>
#include <filesystem>

#include "config_manager.h"
#include "service/TranscodeService.h"

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

    // 初始化 Drogon 客户端
    // 数据库 (Default Write)
    drogon::app().createDbClient("postgresql", 
        cfg.getDbMasterHost(),
        cfg.getDbMasterPort(),
        "lepai_db",      
        "lepai_admin",   
        "lepai_password",
        1,
        "default",
        "default"
    );
    
    // Redis
    drogon::app().createRedisClient(
        cfg.getRedisHost(),
        6379,
        "default",
        ""
    );

    // 实例化服务
    auto workerService = std::make_shared<lepai::worker::service::TranscodeService>();
    
    // 启动服务逻辑
    workerService->start();

    // 运行事件循环
    LOG_INFO << "Video Worker is running...";
    drogon::app().run();

    return 0;
}