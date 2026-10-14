#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "HardwareInfo.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    HardwareInfoProvider hardwareInfo;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("hardwareInfo", &hardwareInfo);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("TaskManagerPlus", "Main");

    return QGuiApplication::exec();
}
