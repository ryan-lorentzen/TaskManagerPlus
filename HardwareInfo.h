#pragma once

#include <QObject>
#include <QStringList>
#include <QList>
#include <memory>

class QThread;

struct MemoryModule {
    quint64 capacityBytes = 0;
    quint16 smbiosMemoryType = 0;
};

struct InstalledMemory {
    quint64 totalBytes = 0;
    QString type;
};

struct GraphicsAdapter {
    QString name;
    QString deviceId;
};

struct HardwareInfo {
    QString cpuName = QStringLiteral("Unknown CPU");
    QString gpuName = QStringLiteral("Unknown GPU");
    QStringList gpuNames;
    InstalledMemory memory;
};

QString memoryTypeName(quint16 smbiosType);
InstalledMemory aggregateMemory(const QList<MemoryModule> &modules);
QString formatInstalledMemory(const InstalledMemory &memory);
QString selectGraphicsAdapter(const QList<GraphicsAdapter> &adapters,
                              const QString &primaryDeviceId);
HardwareInfo queryHardwareInfo();

class HardwareInfoProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString cpuName READ cpuName NOTIFY hardwareInfoChanged)
    Q_PROPERTY(QString gpuName READ gpuName NOTIFY hardwareInfoChanged)
    Q_PROPERTY(QString installedMemory READ installedMemory NOTIFY hardwareInfoChanged)

public:
    explicit HardwareInfoProvider(QObject *parent = nullptr);
    ~HardwareInfoProvider() override;

    QString cpuName() const { return m_info.cpuName; }
    QString gpuName() const { return m_info.gpuName; }
    QString installedMemory() const { return formatInstalledMemory(m_info.memory); }

signals:
    void hardwareInfoChanged();

private:
    HardwareInfo m_info;
    std::unique_ptr<QThread> m_worker;
};
