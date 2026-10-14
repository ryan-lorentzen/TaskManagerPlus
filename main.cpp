#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "HardwareInfo.h"
#include "SystemMetricsProvider.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    HardwareInfoProvider hardwareInfo;
    SystemMetricsProvider systemMetrics;
    QObject::connect(&hardwareInfo, &HardwareInfoProvider::hardwareInfoChanged,
                     &systemMetrics, [&] {
                         systemMetrics.setGraphicsAdapter(hardwareInfo.gpuName(), hardwareInfo.gpuDeviceId());
                     });
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("hardwareInfo", &hardwareInfo);
    engine.rootContext()->setContextProperty("systemMetrics", &systemMetrics);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("TaskManagerPlus", "Main");

    return QGuiApplication::exec();
}
