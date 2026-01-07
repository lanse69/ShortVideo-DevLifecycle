#include <drogon/drogon.h>
#include <QCoreApplication>
#include <QDebug>

#include "config_manager.h"

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
    qInfo() << "DB Host:" << cfg.getDbHost();

    // 配置 Drogon 数据库连接
    drogon::app().createDbClient("postgresql", 
        cfg.getDbHost().toStdString(),
        cfg.getDbPort(),
        "lepai_db",      // DB Name
        "lepai_admin",   // User
        "lepai_password",// Password
        1                // 连接池大小
    );

    // 配置监听端口
    drogon::app().addListener("0.0.0.0", 8080);

    // 运行 Drogon 事件循环
    drogon::app().run();

    return 0;
}