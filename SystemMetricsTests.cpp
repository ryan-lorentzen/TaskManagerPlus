#include "SystemMetricsProvider.h"

#include <QCoreApplication>
#include <QDebug>
#include <cmath>
#include <limits>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    int checks = 0, failures = 0;
    auto check = [&](bool condition, const char *name) {
        ++checks;
        if (!condition) { qCritical() << "FAIL:" << name; ++failures; }
    };
    const CpuTimes baseline{100, 200, 100};
    check(calculateCpuUsage(baseline, {200, 300, 100}) == 0.0, "CPU idle");
    check(calculateCpuUsage(baseline, {150, 250, 150}) == 50.0, "CPU half busy, kernel includes idle");
    check(calculateCpuUsage(baseline, {100, 250, 150}) == 100.0, "CPU fully busy");
    check(!calculateCpuUsage(baseline, baseline), "CPU zero delta");
    check(!calculateCpuUsage(baseline, {99, 300, 200}), "CPU counter regression");
    check(!calculateCpuUsage(baseline, {300, 250, 150}), "CPU invalid idle delta");
    check(!calculateCpuUsage({}, {0, std::numeric_limits<quint64>::max(), 1}), "CPU overflow");

    constexpr quint64 gb = 1024ULL * 1024 * 1024;
    const auto memory = calculateMemoryUsage(64 * gb, 43 * gb);
    check(memory && memory->usedBytes == 21 * gb && memory->totalBytes == 64 * gb, "Physical memory subtraction");
    check(formatMemoryUsed(memory) == "21.0 GB", "Used memory formatting");
    check(formatMemoryUsed(calculateMemoryUsage(4 * gb, 5 * gb)) == QStringLiteral("—"), "Invalid memory unavailable");
    check(!calculateMemoryUsage(0, 0), "Zero memory total");
    check(calculateMemoryUsage(gb, gb)->usedBytes == 0, "Zero memory usage is valid");
    check(std::abs(celsiusToFahrenheit(54) - 129.2) < 0.0001, "Celsius to Fahrenheit");
    check(formatTemperatureValue(54, true) == QStringLiteral("129 °F"), "Fahrenheit display");
    check(formatTemperatureValue({}, false) == QStringLiteral("—"), "Missing temperature display");
    check(formatTemperatureValue(std::numeric_limits<double>::quiet_NaN(), false) == QStringLiteral("—"),
          "Invalid temperature display");
    const QJsonObject temperatures{{"requestId", 7}, {"cpuTemperatureC", 60.125},
                                  {"gpuTemperatureC", 74.875}, {"cpuSampleOffsetMs", 10},
                                  {"gpuSampleOffsetMs", 50}, {"collectionDurationMs", 60}};
    check(freshTemperatureReading(temperatures, true, 7, 70, 0) == 60.125,
          "Fresh CPU preserves raw Celsius without percentage adjustment");
    check(freshTemperatureReading(temperatures, false, 7, 70, 0) == 74.875,
          "Fresh GPU preserves raw Celsius without percentage adjustment");
    check(freshTemperatureReading(temperatures, true, 7, 70, 940) == 60.125,
          "One-interval freshness boundary");
    check(!freshTemperatureReading(temperatures, true, 7, 70, 941), "Expired CPU reading unavailable");
    check(freshTemperatureReading(temperatures, false, 7, 70, 950) == 74.875,
          "GPU freshness independent of earlier CPU acquisition");
    check(!freshTemperatureReading(temperatures, false, 8, 70, 0), "Mismatched request rejected");
    check(!freshTemperatureReading(temperatures, false, 7, 2000, 0), "Late transport reply rejected");
    check(!freshTemperatureReading(temperatures, false, 7, 10, 0), "Impossible future acquisition rejected");
    check(!freshTemperatureReading(temperatures, false, 7, 70, -1), "Invalid age rejected");
    auto missingTemperature = temperatures;
    missingTemperature["cpuTemperatureC"] = QJsonValue::Null;
    check(!freshTemperatureReading(missingTemperature, true, 7, 70, 0), "Missing CPU does not become zero");
    check(freshTemperatureReading(missingTemperature, false, 7, 70, 0) == 74.875,
          "Missing CPU leaves GPU available");
    missingTemperature["gpuSampleOffsetMs"] = QJsonValue::Null;
    check(!freshTemperatureReading(missingTemperature, false, 7, 70, 0), "Missing acquisition metadata rejected");
    check(formatTemperatureValue(74.875, false) == QStringLiteral("75 °C"), "Celsius display rounding only");

    auto instance = [](int pid, int engine, int low = 2, int physical = 0) {
        return QString("pid_%1_luid_0x00000000_0x%2_phys_%3_eng_%4_engtype_3D")
            .arg(pid).arg(low, 8, 16, QLatin1Char('0')).arg(physical).arg(engine);
    };
    const QList<GpuEngineReading> engines = {{instance(1, 0), 30}, {instance(2, 0), 25},
                                            {instance(1, 1), 40}, {instance(1, 0, 3), 100}};
    check(aggregateGpuEngines(engines, 0, 2) == 55.0, "GPU process contributions summed within engine, other adapter ignored");
    check(aggregateGpuEngines({{instance(1, 0), 80}, {instance(2, 0), 60}, {instance(3, 1), 70}}, 0, 2)
              == 100.0, "GPU utilization capped, not sum of different engines");
    check(aggregateGpuEngines({{instance(1, 0), 30}, {instance(1, 0, 2, 1), 40}}, 0, 2) == 40.0,
          "GPU physical engine identities kept separate");
    check(!aggregateGpuEngines({}, 0, 2), "Missing GPU counters unavailable");
    check(aggregateGpuEngines({{instance(1, 0), 0}}, 0, 2) == 0.0, "GPU measured idle is valid");
    check(!aggregateGpuEngines({{"not-an-engine", 100}, {instance(1, 0), -1},
                                {instance(2, 0), std::numeric_limits<double>::quiet_NaN()}}, 0, 2),
          "GPU invalid counters ignored");

    std::deque<UtilizationPoint> history;
    for (int i = 0; i < 100; ++i) appendUtilization(history, i * 1000, double(i));
    check(history.size() == 60 && history.front().timestampMs == 40000
              && history.back().timestampMs == 99000, "Rolling history capacity and order");
    appendUtilization(history, 99000, 100);
    check(history.size() == 60 && history.back().percent == 99, "Duplicate sample rejected");
    appendUtilization(history, 98000, 100);
    check(history.back().percent == 99, "Out-of-order sample rejected");
    appendUtilization(history, 200000, {});
    check(history.size() == 1 && !history.front().percent, "Elapsed window pruning and missing-data gap");
    appendUtilization(history, 201000, 101);
    check(!history.back().percent, "Invalid chart values cannot exceed scale");
    appendUtilization(history, 202127, 20);
    check(history.back().timestampMs == 202127 && history.back().percent == 20,
          "Actual timestamps preserved without interpolation");
    qInfo() << checks << "metrics checks," << failures << "failures";
    return failures == 0 ? 0 : 1;
}
