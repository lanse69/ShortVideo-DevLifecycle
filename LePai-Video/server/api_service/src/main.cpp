#include <drogon/drogon.h>
#include <QCoreApplication>
#include <QDebug>

#include "config_manager.h"
#include "SyncScheduler.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    // 加载配置
    QString configPath = QCoreApplication::applicationDirPath() + "/config.json";
    if (!ConfigManager::instance().loadConfig(configPath)) {
        qCritical() << "Failed to load config from" << configPath;
        return 1;
    }

    auto& cfg = ConfigManager::instance();
    qInfo() << "Starting API Server...";

    // 配置数据库客户端 (读写分离)
    // [Client 1] 主库 (Master) - 用于写入 (Register, Upload, Like)
    qInfo() << "Master DB:" << cfg.getDbMasterHost() << ":" << cfg.getDbMasterPort();
    drogon::app().createDbClient("postgresql", 
        cfg.getDbMasterHost().toStdString(),
        cfg.getDbMasterPort(),
        "lepai_db",      
        "lepai_admin",   
        "lepai_password",
        1,               // 连接池大小
        "default"
    );
    // [Client 2] 从库 (Slave) - 用于读取 (GetFeed, GetUserInfo)
    qInfo() << "Slave DB :" << cfg.getDbSlaveHost() << ":" << cfg.getDbSlavePort();
    drogon::app().createDbClient("postgresql", 
        cfg.getDbSlaveHost().toStdString(),
        cfg.getDbSlavePort(),
        "lepai_db",
        "lepai_admin",
        "lepai_password",
        5,               
        "slave"
    );

    // 配置 Redis 客户端
    qInfo() << "Redis Host:" << cfg.getRedisHost();
    drogon::app().createRedisClient(
        cfg.getRedisHost().toStdString(),
        6379,
        "default",
        ""
    );

    // 启动定时同步任务
    drogon::app().getLoop()->runEvery(10.0, []() {
        SyncScheduler::syncLikesToDB(); 
    });

    // 配置监听端口
    drogon::app().addListener("0.0.0.0", 8080);

    // 运行 Drogon 事件循环
    drogon::app().run();

    return 0;
}
