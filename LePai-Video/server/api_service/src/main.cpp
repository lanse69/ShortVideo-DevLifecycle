// #include <drogon/drogon.h>
// #include <drogon/orm/DbConfig.h>
// #include <QCoreApplication>
// #include <QDebug>

// #include "config_manager.h"
// #include "scheduler/SyncScheduler.h"

// int main(int argc, char *argv[]) {
//     QCoreApplication a(argc, argv);

//     // 加载配置
//     QString configPath = QCoreApplication::applicationDirPath() + "/config.json";
//     if (!ConfigManager::instance().loadConfig(configPath)) {
//         qCritical() << "Failed to load config from" << configPath;
//         return 1;
//     }

//     auto& cfg = ConfigManager::instance();
//     qInfo() << "Starting API Server...";

//     // [Client 1] 主库配置 (Master) - 用于写入
//     qInfo() << "Master DB:" << cfg.getDbMasterHost() << ":" << cfg.getDbMasterPort();
//     drogon::orm::PostgresConfig masterConfig;
//     masterConfig.host = cfg.getDbMasterHost().toStdString();
//     masterConfig.port = cfg.getDbMasterPort();
//     masterConfig.databaseName = "lepai_db";
//     masterConfig.username = "lepai_admin";
//     masterConfig.password = "lepai_password";
//     masterConfig.connectionNumber = 1;
//     masterConfig.name = "default"; // 客户端名称
//     drogon::app().addDbClient(masterConfig);

//     // [Client 2] 从库配置 (Slave) - 用于读取
//     qInfo() << "Slave DB :" << cfg.getDbSlaveHost() << ":" << cfg.getDbSlavePort();
//     drogon::orm::PostgresConfig slaveConfig;
//     slaveConfig.host = cfg.getDbSlaveHost().toStdString();
//     slaveConfig.port = cfg.getDbSlavePort();
//     slaveConfig.databaseName = "lepai_db";
//     slaveConfig.username = "lepai_admin";
//     slaveConfig.password = "lepai_password";
//     slaveConfig.connectionNumber = 5;
//     slaveConfig.name = "slave"; // 客户端名称
//     drogon::app().addDbClient(slaveConfig);

//     // 配置 Redis 客户端
//     qInfo() << "Redis Host:" << cfg.getRedisHost();
//     drogon::app().createRedisClient(
//         cfg.getRedisHost().toStdString(),
//         6379,
//         "default",
//         ""
//     );

//     // 启动定时同步 (每10秒同步一次)
//     drogon::app().registerBeginningAdvice([]() {
//         drogon::app().getLoop()->runEvery(10.0, []() {
//             lepai::scheduler::SyncScheduler::syncLikesToDB();
            
//             if (!drogon::app().getDbClient("slave")) {
//                 qWarning() << "DB Client 'slave' not found!";
//             }
//             if (!drogon::app().getDbClient("default")) {
//                 qWarning() << "DB Client 'default' not found!";
//             }
//         });
//     });

//     // 配置监听端口
//     drogon::app().addListener("0.0.0.0", 8080);

//     // 运行 Drogon 事件循环
//     drogon::app().run();

//     return 0;
// }

#include <drogon/drogon.h>
#include <QCoreApplication>
#include <QDebug>

#include "config_manager.h"
#include "scheduler/SyncScheduler.h"

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

    // [Client 1] 主库
    qInfo() << "Master DB:" << cfg.getDbMasterHost() << ":" << cfg.getDbMasterPort();
    drogon::app().createDbClient("postgresql", 
        cfg.getDbMasterHost().toStdString(),
        cfg.getDbMasterPort(),
        "lepai_db",      
        "lepai_admin",   
        "lepai_password",
        1,
        "default",
        "default"
    );

    // [Client 2] 从库
    qInfo() << "Slave DB :" << cfg.getDbSlaveHost() << ":" << cfg.getDbSlavePort();
    drogon::app().createDbClient("postgresql", 
        cfg.getDbSlaveHost().toStdString(),
        cfg.getDbSlavePort(),
        "lepai_db",
        "lepai_admin",
        "lepai_password",
        5,
        "default",
        "slave"
    );

    // 配置 Redis
    qInfo() << "Redis Host:" << cfg.getRedisHost();
    drogon::app().createRedisClient(
        cfg.getRedisHost().toStdString(),
        6379,
        "default",
        ""
    );

    // 启动定时同步 (每10秒同步一次)
    drogon::app().registerBeginningAdvice([]() {
        drogon::app().getLoop()->runEvery(10.0, []() {
            qInfo() << "now to flush.";
            lepai::scheduler::SyncScheduler::syncLikesToDB();
            
            if (!drogon::app().getDbClient("slave")) {
                qWarning() << "DB Client 'slave' not found!";
            }
            if (!drogon::app().getDbClient("default")) {
                qWarning() << "DB Client 'default' not found!";
            }
        });
    });

    drogon::app().addListener("0.0.0.0", 8080);
    drogon::app().run();

    return 0;
}