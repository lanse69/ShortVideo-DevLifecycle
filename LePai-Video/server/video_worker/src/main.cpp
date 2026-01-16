#include <drogon/drogon.h>
#include <filesystem>
#include <vector>
#include <iostream>

#include "config_manager.h"
#include "service/TranscodeService.h"

int main(int argc, char *argv[]) {
    // 获取可执行文件所在的目录
    std::filesystem::path exeDir;
    try {
        exeDir = std::filesystem::canonical("/proc/self/exe").parent_path();
    } catch (...) {
        exeDir = std::filesystem::current_path();
    }

    // 配置文件搜索路径
    std::vector<std::filesystem::path> searchPaths;

    // 命令行参数 (允许 ./api_service /tmp/my_config.json)
    if (argc > 1) {
        searchPaths.push_back(argv[1]);
    }

    // 当前运行目录 (./config.json)
    searchPaths.push_back(std::filesystem::current_path() / "config.json");

    // 可执行文件同级目录
    searchPaths.push_back(exeDir / "config.json");

    // 部署结构的配置目录 (../config/config.json)
    searchPaths.push_back(exeDir.parent_path() / "config" / "config.json");

    // 生产环境默认位置
    searchPaths.push_back("/opt/lepai/config/config.json");

    std::string configPathStr;
    for (const auto& path : searchPaths) {
        std::error_code ec;
        if (std::filesystem::exists(path, ec) && std::filesystem::is_regular_file(path, ec)) {
            configPathStr = std::filesystem::absolute(path).string();
            break;
        }
    }

    if (configPathStr.empty()) {
        LOG_ERROR << "CRITICAL: Config file 'config.json' not found!";
        LOG_ERROR << "Searched locations:";
        for (const auto& p : searchPaths) {
            LOG_ERROR << " - " << p.string();
        }
        return 1;
    }

    LOG_INFO << "Loading configuration from: " << configPathStr;

    if (!ConfigManager::instance().loadConfig(configPathStr)) {
        LOG_ERROR << "Failed to parse config file: " << configPathStr;
        return 1;
    }

    auto& cfg = ConfigManager::instance();

    drogon::app().setLogLocalTime(true); 

    // 数据库 (Default Write)
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
    
    // Redis
    drogon::app().createRedisClient(
        cfg.getRedisHost(),
        cfg.getRedisPort(),
        "default",
        cfg.getRedisPassword()
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