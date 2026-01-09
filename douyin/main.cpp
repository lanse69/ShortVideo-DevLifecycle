#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include "musicviewmodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // 或者创建全局实例
    MusicViewModel *musicVM = new MusicViewModel();
    engine.rootContext()->setContextProperty("musicViewModel", musicVM);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("douyin", "Main");
    app.setWindowIcon(QIcon(":/icon/icons/douyin.jpg"));

    return app.exec();
}
