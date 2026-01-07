#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCoreApplication>

#include "ConfigManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 初始化配置
    QString configPath = QCoreApplication::applicationDirPath() + "/config.json";
    ConfigManager::instance().loadConfig(configPath);

    QQmlApplicationEngine engine;

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("LePaiClient", "Main");

    return app.exec();
}