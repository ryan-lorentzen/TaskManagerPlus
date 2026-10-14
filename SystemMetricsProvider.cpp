#include "SystemMetricsProvider.h"

#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QThread>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <limits>
#include <mutex>
#include <vector>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dxgi1_2.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <wrl/client.h>
#endif

std::optional<double> calculateCpuUsage(const CpuTimes &previous, const CpuTimes &current)
{
    if (current.idle < previous.idle || current.kernel < previous.kernel
        || current.user < previous.user)
        return {};
    const quint64 idle = current.idle - previous.idle;
    const quint64 kernel = current.kernel - previous.kernel;
    const quint64 user = current.user - previous.user;
    if (kernel > std::numeric_limits<quint64>::max() - user)
        return {};
    const quint64 total = kernel + user;
    if (total == 0 || idle > total)
        return {};
    return std::clamp(100.0 * double(total - idle) / double(total), 0.0, 100.0);
}

std::optional<PhysicalMemoryUsage> calculateMemoryUsage(quint64 total, quint64 available)
{
    if (total == 0 || available > total)
        return {};
    return PhysicalMemoryUsage{total - available, total};
}

std::optional<double> aggregateGpuEngines(const QList<GpuEngineReading> &readings,
                                        quint32 luidHigh, quint32 luidLow)
{
    // Instance names are supplied by PDH, not guessed or tied to a process list.
    static const QRegularExpression instancePattern(
        QStringLiteral("^pid_\\d+_luid_0x([0-9a-f]+)_0x([0-9a-f]+)_phys_(\\d+)_eng_(\\d+)_engtype_.+$"),
        QRegularExpression::CaseInsensitiveOption);
    QMap<QString, double> engines;
    for (const auto &reading : readings) {
        if (!std::isfinite(reading.percent) || reading.percent < 0)
            continue;
        const auto match = instancePattern.match(reading.instance);
        if (!match.hasMatch())
            continue;
        bool highOk = false;
        bool lowOk = false;
        const quint32 high = match.captured(1).toUInt(&highOk, 16);
        const quint32 low = match.captured(2).toUInt(&lowOk, 16);
        if (!highOk || !lowOk || high != luidHigh || low != luidLow)
            continue;
        // All processes sharing one physical engine contribute to its busy time.
        const QString key = match.captured(3) + u'/' + match.captured(4);
        engines[key] = std::min(100.0, engines.value(key) + reading.percent);
    }
    if (engines.isEmpty())
        return {};
    // Task Manager uses the busiest engine as its overall utilization indicator.
    return *std::max_element(engines.cbegin(), engines.cend());
}

double celsiusToFahrenheit(double celsius)
{
    return celsius * 9.0 / 5.0 + 32.0;
}

QString formatTemperatureValue(std::optional<double> celsius, bool fahrenheit)
{
    if (!celsius || !std::isfinite(*celsius))
        return QStringLiteral("—");
    return QString::number(std::round(fahrenheit ? celsiusToFahrenheit(*celsius) : *celsius), 'f', 0)
           + (fahrenheit ? QStringLiteral(" °F") : QStringLiteral(" °C"));
}

std::optional<double> freshTemperatureReading(const QJsonObject &response, bool cpu,
                                             qint64 expectedRequestId, qint64 roundTripMs,
                                             qint64 timeSinceResponseMs)
{
    // Each reading expires after one sampling interval. Offset timestamps let a
    // slow GPU update expire the older CPU reading independently (and vice versa).
    constexpr qint64 maximumAgeMs = 1000;
    if (expectedRequestId < 0 || response.value("requestId").toInteger(-1) != expectedRequestId
        || roundTripMs < 0 || timeSinceResponseMs < 0)
        return {};
    const qint64 duration = response.value("collectionDurationMs").toInteger(-1);
    const qint64 offset = response.value(cpu ? "cpuSampleOffsetMs" : "gpuSampleOffsetMs").toInteger(-1);
    if (offset < 0 || duration < offset || duration - roundTripMs > 5
        || offset - roundTripMs > 5)
        return {};
    // Treat transport/startup delay conservatively; never call a delayed response fresh.
    const qint64 ageAtResponse = std::max<qint64>(0, roundTripMs - offset);
    if (ageAtResponse > maximumAgeMs || timeSinceResponseMs > maximumAgeMs - ageAtResponse)
        return {};
    const auto value = response.value(cpu ? "cpuTemperatureC" : "gpuTemperatureC");
    if (!value.isDouble()) return {};
    const double celsius = value.toDouble();
    return std::isfinite(celsius) && celsius > 0 && celsius < 150
               ? std::optional<double>(celsius) : std::nullopt;
}

QString formatMemoryUsed(std::optional<PhysicalMemoryUsage> memory)
{
    if (!memory || memory->totalBytes == 0 || memory->usedBytes > memory->totalBytes)
        return QStringLiteral("—");
    return QString::number(double(memory->usedBytes) / (1024.0 * 1024 * 1024), 'f', 1)
           + QStringLiteral(" GB");
}

void appendUtilization(std::deque<UtilizationPoint> &history, qint64 timestampMs,
                       std::optional<double> percent)
{
    if (!history.empty() && timestampMs <= history.back().timestampMs)
        return; // Never fabricate catch-up or duplicate samples.
    if (percent && (!std::isfinite(*percent) || *percent < 0 || *percent > 100))
        percent.reset();
    history.push_back({timestampMs, percent});
    while (history.size() > 60 || history.front().timestampMs < timestampMs - 60000)
        history.pop_front();
}

namespace {
QVariant optionalVariant(std::optional<double> value)
{
    return value ? QVariant(*value) : QVariant();
}

QVariantList chartPoints(const std::deque<UtilizationPoint> &history)
{
    QVariantList points;
    points.reserve(qsizetype(history.size()));
    for (const auto &point : history)
        points.append(QVariantMap{{QStringLiteral("timestampMs"), point.timestampMs},
                                  {QStringLiteral("percent"), optionalVariant(point.percent)}});
    return points;
}

// This bridge is only for temperatures. It is launched once, not once per sample.
class TemperatureBridge
{
public:
    TemperatureBridge()
    {
        m_clock.start();
#ifdef Q_OS_WIN
        const QString executable = QCoreApplication::applicationDirPath()
                                   + QStringLiteral("/sensors/TaskManager.SensorBridge.exe");
        if (!QFileInfo::exists(executable)) {
            qWarning() << "Temperature helper not built; temperatures are unavailable";
            return;
        }
        m_process.setProgram(executable);
        m_process.setProcessChannelMode(QProcess::SeparateChannels);
        m_process.start();
        if (!m_process.waitForStarted(2000))
            qWarning() << "Temperature helper could not start; native metrics remain available";
#endif
    }

    ~TemperatureBridge()
    {
        if (m_process.state() != QProcess::NotRunning) {
            m_process.closeWriteChannel(); // EOF tells the helper to close its Computer.
            if (!m_process.waitForFinished(2000)) {
                m_process.kill();
                m_process.waitForFinished(1000);
            }
        }
    }

    void sample(SystemMetrics &metrics, const QString &gpuName, const QString &deviceId)
    {
        if (m_process.state() == QProcess::NotRunning)
            return;
        consume();
        if (!m_pending) {
            ++m_requestId;
            const auto request = QJsonDocument(QJsonObject{{"gpuName", gpuName},
                                                           {"gpuDeviceId", deviceId},
                                                           {"requestId", m_requestId}})
                                     .toJson(QJsonDocument::Compact) + '\n';
            m_requestClock.start();
            m_requestGpuName = gpuName;
            m_requestGpuDeviceId = deviceId;
            if (m_process.write(request) < 0) return;
            m_process.waitForBytesWritten(25);
            m_pending = true;
        }
        // Slow/failed sensors cannot hold up CPU, GPU counters, or memory.
        m_process.waitForReadyRead(100);
        consume();
        if (m_lastResponse >= 0) {
            const qint64 age = m_clock.elapsed() - m_lastResponse;
            metrics.cpuTemperatureC = freshTemperatureReading(m_response, true, m_responseRequestId,
                                                              m_responseRoundTripMs, age);
            if (m_responseGpuName == gpuName && m_responseGpuDeviceId == deviceId)
                metrics.gpuTemperatureC = freshTemperatureReading(m_response, false, m_responseRequestId,
                                                                  m_responseRoundTripMs, age);
        }
    }

private:
    void consume()
    {
        m_buffer += m_process.readAllStandardOutput();
        m_errorBuffer += m_process.readAllStandardError();
        if (m_errorBuffer.size() > 65536) m_errorBuffer.clear();
        qsizetype errorEnd;
        while ((errorEnd = m_errorBuffer.indexOf('\n')) >= 0) {
            const QString message = QString::fromUtf8(m_errorBuffer.left(errorEnd)).trimmed().left(512);
            m_errorBuffer.remove(0, errorEnd + 1);
            if (!message.isEmpty() && m_reportedErrors.size() < 32 && !m_reportedErrors.contains(message)) {
                m_reportedErrors.insert(message);
                qWarning().noquote() << "Temperature helper:" << message;
            }
        }
        if (m_buffer.size() > 65536) {
            m_buffer.clear();
            m_response = {};
            return;
        }
        qsizetype end;
        while ((end = m_buffer.indexOf('\n')) >= 0) {
            const auto document = QJsonDocument::fromJson(m_buffer.left(end));
            m_buffer.remove(0, end + 1);
            if (!document.isObject())
                continue;
            const auto object = document.object();
            if (!m_pending || object.value("requestId").toInteger(-1) != m_requestId)
                continue; // Ignore an old or unsolicited reply, not the current in-flight request.
            m_response = object;
            m_responseRequestId = m_requestId;
            m_responseRoundTripMs = m_requestClock.elapsed();
            m_responseGpuName = m_requestGpuName;
            m_responseGpuDeviceId = m_requestGpuDeviceId;
            m_lastResponse = m_clock.elapsed();
            m_pending = false;
        }
    }

    QProcess m_process;
    QByteArray m_buffer;
    QByteArray m_errorBuffer;
    QSet<QString> m_reportedErrors;
    QElapsedTimer m_clock;
    QElapsedTimer m_requestClock;
    bool m_pending = false;
    qint64 m_lastResponse = -1;
    qint64 m_requestId = 0;
    qint64 m_responseRequestId = -1;
    qint64 m_responseRoundTripMs = 0;
    QJsonObject m_response;
    QString m_requestGpuName;
    QString m_requestGpuDeviceId;
    QString m_responseGpuName;
    QString m_responseGpuDeviceId;
};

#ifdef Q_OS_WIN
using Microsoft::WRL::ComPtr;

quint64 fileTimeValue(const FILETIME &time)
{
    return (quint64(time.dwHighDateTime) << 32) | time.dwLowDateTime;
}

QString displayConfigDeviceId(const LUID &luid)
{
    DISPLAYCONFIG_ADAPTER_NAME adapter{};
    adapter.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADAPTER_NAME;
    adapter.header.size = sizeof(adapter);
    adapter.header.adapterId = luid;
    if (DisplayConfigGetDeviceInfo(&adapter.header) != ERROR_SUCCESS)
        return {};
    // Device interface path: \\?\PCI#VEN_...#instance#{GUID} -> PCI\VEN_...\instance.
    QString path = QString::fromWCharArray(adapter.adapterDevicePath);
    if (path.startsWith(QStringLiteral("\\\\?\\"))) path.remove(0, 4);
    const auto guid = path.indexOf(QStringLiteral("#{"));
    if (guid >= 0) path.truncate(guid);
    return path.replace(u'#', u'\\');
}

class NativeSampler
{
public:
    NativeSampler()
    {
        if (!m_cpuSupported)
            qWarning() << "CPU usage unavailable: GetSystemTimes cannot represent all processor groups";
    }
    ~NativeSampler() { closeGpuQuery(); }

    SystemMetrics sample(const QString &gpuName, const QString &gpuDeviceId, qint64 timestampMs)
    {
        SystemMetrics metrics;
        metrics.timestampMs = timestampMs;
        FILETIME idle{}, kernel{}, user{};
        if (m_cpuSupported && GetSystemTimes(&idle, &kernel, &user)) {
            const CpuTimes current{fileTimeValue(idle), fileTimeValue(kernel), fileTimeValue(user)};
            if (m_previous) metrics.cpuUsagePercent = calculateCpuUsage(*m_previous, current);
            m_previous = current;
        } else {
            m_previous.reset();
        }

        MEMORYSTATUSEX memory{};
        memory.dwLength = sizeof(memory);
        if (GlobalMemoryStatusEx(&memory))
            metrics.memory = calculateMemoryUsage(memory.ullTotalPhys, memory.ullAvailPhys);

        if (gpuName != m_gpuName || gpuDeviceId != m_gpuDeviceId) {
            m_gpuName = gpuName;
            m_gpuDeviceId = gpuDeviceId;
            initializeGpu();
        }
        if (m_query && PdhCollectQueryData(m_query) == ERROR_SUCCESS) {
            if (m_gpuBaseline) metrics.gpuUsagePercent = readGpu();
            m_gpuBaseline = true;
        } else {
            m_gpuBaseline = false;
        }
        return metrics;
    }

private:
    void closeGpuQuery()
    {
        if (m_query) PdhCloseQuery(m_query);
        m_query = nullptr;
        m_counter = nullptr;
        m_gpuBaseline = false;
    }

    void initializeGpu()
    {
        closeGpuQuery();
        if (m_gpuName.isEmpty() || m_gpuName == QStringLiteral("Unknown GPU"))
            return;
        ComPtr<IDXGIFactory1> factory;
        if (FAILED(CreateDXGIFactory1(IID_IDXGIFactory1,
                                     reinterpret_cast<void **>(factory.GetAddressOf()))))
            return;
        QList<DXGI_ADAPTER_DESC1> exactMatches;
        QList<DXGI_ADAPTER_DESC1> nameMatches;
        for (UINT index = 0; ; ++index) {
            ComPtr<IDXGIAdapter1> adapter;
            const HRESULT hr = factory->EnumAdapters1(index, adapter.GetAddressOf());
            if (hr == DXGI_ERROR_NOT_FOUND) break;
            if (FAILED(hr)) return;
            DXGI_ADAPTER_DESC1 description{};
            if (FAILED(adapter->GetDesc1(&description))
                || (description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
                continue;
            if (!m_gpuDeviceId.isEmpty()
                && displayConfigDeviceId(description.AdapterLuid)
                       .compare(m_gpuDeviceId, Qt::CaseInsensitive) == 0)
                exactMatches.append(description);
            if (QString::fromWCharArray(description.Description).trimmed() == m_gpuName)
                nameMatches.append(description);
        }
        // Do not silently measure a different GPU when names/IDs are ambiguous.
        if (exactMatches.size() == 1) m_luid = exactMatches.first().AdapterLuid;
        else if (nameMatches.size() == 1) m_luid = nameMatches.first().AdapterLuid;
        else return;

        if (PdhOpenQueryW(nullptr, 0, &m_query) != ERROR_SUCCESS
            || PdhAddEnglishCounterW(m_query, L"\\GPU Engine(*)\\Utilization Percentage",
                                     0, &m_counter) != ERROR_SUCCESS) {
            closeGpuQuery();
            qWarning() << "GPU Engine counters unavailable for the selected adapter";
        }
    }

    std::optional<double> readGpu()
    {
        DWORD bytes = 0;
        DWORD count = 0;
        PDH_STATUS status = PdhGetFormattedCounterArrayW(
            m_counter, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100, &bytes, &count, nullptr);
        if (status != PDH_MORE_DATA || bytes == 0 || bytes > 8 * 1024 * 1024)
            return {};
        // Instances can appear between size and data calls. Retry boundedly if needed.
        for (int attempt = 0; attempt < 3; ++attempt) {
            std::vector<unsigned char> buffer(bytes);
            auto *items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W *>(buffer.data());
            status = PdhGetFormattedCounterArrayW(m_counter, PDH_FMT_DOUBLE | PDH_FMT_NOCAP100,
                                                  &bytes, &count, items);
            if (status == PDH_MORE_DATA) {
                if (bytes > 8 * 1024 * 1024) return {};
                continue;
            }
            if (status != ERROR_SUCCESS) return {};
            QList<GpuEngineReading> readings;
            readings.reserve(count);
            for (DWORD index = 0; index < count; ++index) {
                const auto &value = items[index].FmtValue;
                if (items[index].szName && (value.CStatus == PDH_CSTATUS_VALID_DATA
                                           || value.CStatus == PDH_CSTATUS_NEW_DATA))
                    readings.append({QString::fromWCharArray(items[index].szName), value.doubleValue});
            }
            return aggregateGpuEngines(readings, quint32(m_luid.HighPart), m_luid.LowPart);
        }
        return {};
    }

    std::optional<CpuTimes> m_previous;
    bool m_cpuSupported = GetActiveProcessorGroupCount() == 1;
    QString m_gpuName;
    QString m_gpuDeviceId;
    LUID m_luid{};
    PDH_HQUERY m_query = nullptr;
    PDH_HCOUNTER m_counter = nullptr;
    bool m_gpuBaseline = false;
};
#else
class NativeSampler
{
public:
    SystemMetrics sample(const QString &, const QString &, qint64 timestampMs)
    {
        SystemMetrics metrics;
        metrics.timestampMs = timestampMs;
        return metrics;
    }
};
#endif
} // namespace

struct SystemMetricsProvider::State {
    std::mutex mutex;
    std::condition_variable wake;
    bool stopping = false;
    bool deliveryQueued = false;
    QString gpuName;
    QString gpuDeviceId;
    SystemMetrics latest;
    std::deque<UtilizationPoint> cpuHistory;
    std::deque<UtilizationPoint> gpuHistory;
};

SystemMetricsProvider::SystemMetricsProvider(QObject *parent)
    : QObject(parent), m_state(std::make_unique<State>())
{
    m_worker.reset(QThread::create([this] {
        NativeSampler native;
        TemperatureBridge temperatures;
        QElapsedTimer clock;
        clock.start();
        auto next = std::chrono::steady_clock::now();
        while (true) {
            QString gpuName, gpuDeviceId;
            {
                std::lock_guard<std::mutex> lock(m_state->mutex);
                if (m_state->stopping) break;
                gpuName = m_state->gpuName;
                gpuDeviceId = m_state->gpuDeviceId;
            }
            SystemMetrics metrics = native.sample(gpuName, gpuDeviceId, clock.elapsed());
            temperatures.sample(metrics, gpuName, gpuDeviceId);
            bool queueDelivery = false;
            {
                std::lock_guard<std::mutex> lock(m_state->mutex);
                m_state->latest = metrics;
                appendUtilization(m_state->cpuHistory, metrics.timestampMs, metrics.cpuUsagePercent);
                appendUtilization(m_state->gpuHistory, metrics.timestampMs, metrics.gpuUsagePercent);
                queueDelivery = !m_state->deliveryQueued;
                m_state->deliveryQueued = true;
            }
            if (queueDelivery) {
                QMetaObject::invokeMethod(this, [this] {
                    {
                        std::lock_guard<std::mutex> lock(m_state->mutex);
                        m_metrics = m_state->latest;
                        m_cpuHistory = m_state->cpuHistory;
                        m_gpuHistory = m_state->gpuHistory;
                        m_state->deliveryQueued = false;
                    }
                    emit snapshotChanged();
                }, Qt::QueuedConnection);
            }
            next += std::chrono::seconds(1);
            if (next <= std::chrono::steady_clock::now())
                next = std::chrono::steady_clock::now() + std::chrono::seconds(1);
            std::unique_lock<std::mutex> lock(m_state->mutex);
            m_state->wake.wait_until(lock, next, [this] { return m_state->stopping; });
        }
    }));
    m_worker->start();
}

SystemMetricsProvider::~SystemMetricsProvider()
{
    {
        std::lock_guard<std::mutex> lock(m_state->mutex);
        m_state->stopping = true;
    }
    m_state->wake.notify_one();
    m_worker->wait();
}

void SystemMetricsProvider::setGraphicsAdapter(const QString &name, const QString &deviceId)
{
    std::lock_guard<std::mutex> lock(m_state->mutex);
    m_state->gpuName = name;
    m_state->gpuDeviceId = deviceId;
}

QVariant SystemMetricsProvider::cpuUsagePercent() const { return optionalVariant(m_metrics.cpuUsagePercent); }
QVariant SystemMetricsProvider::gpuUsagePercent() const { return optionalVariant(m_metrics.gpuUsagePercent); }
QVariant SystemMetricsProvider::cpuTemperatureC() const { return optionalVariant(m_metrics.cpuTemperatureC); }
QVariant SystemMetricsProvider::gpuTemperatureC() const { return optionalVariant(m_metrics.gpuTemperatureC); }
QString SystemMetricsProvider::memoryUsedText() const { return formatMemoryUsed(m_metrics.memory); }
double SystemMetricsProvider::memoryUsedRatio() const
{
    return m_metrics.memory ? double(m_metrics.memory->usedBytes) / m_metrics.memory->totalBytes : 0;
}
QString SystemMetricsProvider::memorySummaryText() const
{
    if (!m_metrics.memory) return QStringLiteral("Memory usage unavailable");
    return QString::number(memoryUsedRatio() * 100, 'f', 0) + QStringLiteral("% of ")
           + QString::number(double(m_metrics.memory->totalBytes) / (1024.0 * 1024 * 1024), 'f', 1)
           + QStringLiteral(" GB in use");
}
QVariantList SystemMetricsProvider::cpuHistory() const { return chartPoints(m_cpuHistory); }
QVariantList SystemMetricsProvider::gpuHistory() const { return chartPoints(m_gpuHistory); }
QString SystemMetricsProvider::temperatureText(const QVariant &celsius, bool fahrenheit) const
{
    bool ok = false;
    const double value = celsius.toDouble(&ok);
    return formatTemperatureValue(celsius.isNull() || !ok ? std::nullopt : std::optional<double>(value),
                                  fahrenheit);
}
