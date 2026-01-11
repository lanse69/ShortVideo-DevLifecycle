#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCoreApplication>
#include <QQmlContext>
#include "videoaudiomerger.h"
#include "fileutils.h"
#include "ConfigManager.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // 初始化配置
    QString configPath = QCoreApplication::applicationDirPath() + "/config.json";
    ConfigManager::instance().loadConfig(configPath);

    QQmlApplicationEngine engine;

    // 或者创建全局实例
    VideoAudioMerger *vaMerger = new VideoAudioMerger();
    engine.rootContext()->setContextProperty("videoaudiomerger",vaMerger);

    FileUtils *fU = new FileUtils();
    engine.rootContext()->setContextProperty("fileutils",fU);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.loadFromModule("LePaiClient", "Main");

    return app.exec();
}
