#pragma once

#include <QObject>
#include <QVariantList>
#include <QJsonObject>
#include <deque>
#include <memory>
#include <optional>

class QThread;

struct CpuTimes {
    quint64 idle = 0;
    quint64 kernel = 0; // Includes idle time on Windows.
    quint64 user = 0;
};

struct PhysicalMemoryUsage {
    quint64 usedBytes = 0;
    quint64 totalBytes = 0;
};

struct SystemMetrics {
    qint64 timestampMs = 0; // Monotonic elapsed time, independent of GUI delivery.
    std::optional<double> cpuUsagePercent;
    std::optional<double> gpuUsagePercent;
    std::optional<double> cpuTemperatureC;
    std::optional<double> gpuTemperatureC;
    std::optional<PhysicalMemoryUsage> memory;
};

struct UtilizationPoint {
    qint64 timestampMs;
    std::optional<double> percent; // Missing readings produce a gap, never a fake zero.
};

struct GpuEngineReading {
    QString instance;
    double percent;
};

std::optional<double> calculateCpuUsage(const CpuTimes &previous, const CpuTimes &current);
std::optional<PhysicalMemoryUsage> calculateMemoryUsage(quint64 total, quint64 available);
std::optional<double> aggregateGpuEngines(const QList<GpuEngineReading> &readings,
                                        quint32 luidHigh, quint32 luidLow);
double celsiusToFahrenheit(double celsius);
QString formatTemperatureValue(std::optional<double> celsius, bool fahrenheit);
std::optional<double> freshTemperatureReading(const QJsonObject &response, bool cpu,
                                             qint64 expectedRequestId, qint64 roundTripMs,
                                             qint64 timeSinceResponseMs);
QString formatMemoryUsed(std::optional<PhysicalMemoryUsage> memory);
void appendUtilization(std::deque<UtilizationPoint> &history, qint64 timestampMs,
                       std::optional<double> percent);

class SystemMetricsProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant cpuUsagePercent READ cpuUsagePercent NOTIFY snapshotChanged)
    Q_PROPERTY(QVariant gpuUsagePercent READ gpuUsagePercent NOTIFY snapshotChanged)
    Q_PROPERTY(QVariant cpuTemperatureC READ cpuTemperatureC NOTIFY snapshotChanged)
    Q_PROPERTY(QVariant gpuTemperatureC READ gpuTemperatureC NOTIFY snapshotChanged)
    Q_PROPERTY(QString memoryUsedText READ memoryUsedText NOTIFY snapshotChanged)
    Q_PROPERTY(QString memorySummaryText READ memorySummaryText NOTIFY snapshotChanged)
    Q_PROPERTY(double memoryUsedRatio READ memoryUsedRatio NOTIFY snapshotChanged)
    Q_PROPERTY(QVariantList cpuHistory READ cpuHistory NOTIFY snapshotChanged)
    Q_PROPERTY(QVariantList gpuHistory READ gpuHistory NOTIFY snapshotChanged)
    Q_PROPERTY(qint64 sampleTimeMs READ sampleTimeMs NOTIFY snapshotChanged)

public:
    explicit SystemMetricsProvider(QObject *parent = nullptr);
    ~SystemMetricsProvider() override;

    void setGraphicsAdapter(const QString &name, const QString &deviceId);
    QVariant cpuUsagePercent() const;
    QVariant gpuUsagePercent() const;
    QVariant cpuTemperatureC() const;
    QVariant gpuTemperatureC() const;
    QString memoryUsedText() const;
    QString memorySummaryText() const;
    double memoryUsedRatio() const;
    QVariantList cpuHistory() const;
    QVariantList gpuHistory() const;
    qint64 sampleTimeMs() const { return m_metrics.timestampMs; }
    Q_INVOKABLE QString temperatureText(const QVariant &celsius, bool fahrenheit) const;

signals:
    void snapshotChanged();

private:
    struct State;
    std::unique_ptr<State> m_state;
    std::unique_ptr<QThread> m_worker;
    SystemMetrics m_metrics;
    std::deque<UtilizationPoint> m_cpuHistory;
    std::deque<UtilizationPoint> m_gpuHistory;
};
