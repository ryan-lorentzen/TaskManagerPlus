#include "HardwareInfo.h"

#include <QCoreApplication>
#include <QDebug>
#include <limits>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    int failures = 0;
    auto check = [&failures](bool condition, const char *description) {
        if (!condition) {
            qCritical() << "FAIL:" << description;
            ++failures;
        }
    };
    constexpr quint64 gb = 1024ULL * 1024 * 1024;
    check(memoryTypeName(24) == "DDR3", "SMBIOS DDR3");
    check(memoryTypeName(26) == "DDR4", "SMBIOS DDR4");
    check(memoryTypeName(34) == "DDR5", "SMBIOS DDR5");
    check(memoryTypeName(30) == "LPDDR4", "SMBIOS LPDDR4");
    check(memoryTypeName(0).isEmpty() && memoryTypeName(2).isEmpty()
              && memoryTypeName(65535).isEmpty(), "Unknown types are not guessed");

    const auto memory = aggregateMemory({{32 * gb, 26}, {32 * gb, 26}});
    check(memory.totalBytes == 64 * gb && memory.type == "DDR4", "Module aggregation");
    check(formatInstalledMemory(memory) == "64 GB DDR4", "Binary GB formatting");
    check(formatInstalledMemory({3 * gb / 2, {}}) == "1.5 GB", "Fractional GB formatting");
    check(formatInstalledMemory(aggregateMemory({{16 * gb, 0}, {16 * gb, 0}}))
              == "32 GB", "Unknown type preserves capacity");
    check(formatInstalledMemory(aggregateMemory({{16 * gb, 26}, {16 * gb, 0}}))
              == "32 GB", "Partially unknown type preserves capacity");
    check(formatInstalledMemory(aggregateMemory({{16 * gb, 26}, {16 * gb, 34}}))
              == "32 GB", "Mixed types do not claim a single type");
    check(aggregateMemory({{0, 26}, {16 * gb, 26}}).totalBytes == 0,
          "Missing module capacity does not produce a misleading partial total");
    check(aggregateMemory({{std::numeric_limits<quint64>::max(), 26}, {gb, 26}})
              .totalBytes == 0, "Capacity overflow handled");
    check(formatInstalledMemory(aggregateMemory({})) == "Unknown RAM", "No modules fallback");

    const QList<GraphicsAdapter> adapters = {{"Z Display GPU", "PCI\\PRIMARY"},
                                             {"A Other GPU", "PCI\\SECONDARY"},
                                             {"", "PCI\\EMPTY"}};
    check(selectGraphicsAdapter(adapters, "pci\\primary") == "Z Display GPU",
          "Primary display adapter chosen by case-insensitive device ID");
    check(selectGraphicsAdapter(adapters, {}) == "A Other GPU",
          "Predictable multiple GPU fallback");
    check(selectGraphicsAdapter({adapters[1], adapters[0]}, "UNMATCHED") == "A Other GPU",
          "Fallback independent of enumeration order");
    check(selectGraphicsAdapter({}, {}) == "Unknown GPU", "No GPUs fallback");
    check(selectGraphicsAdapter({{"  GPU Name  ", {}}}, {}) == "GPU Name",
          "GPU names trimmed");

    qInfo() << (failures == 0 ? "All 19 hardware-info checks passed" : "Hardware-info checks failed");
    return failures == 0 ? 0 : 1;
}
