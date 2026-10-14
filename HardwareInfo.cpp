#include "HardwareInfo.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QThread>
#include <limits>
#include <utility>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wbemidl.h>
#include <wrl/client.h>
#endif

QString memoryTypeName(quint16 smbiosType)
{
    // SMBIOS types, not the older Win32_PhysicalMemory.MemoryType enumeration.
    switch (smbiosType) {
    case 18: return QStringLiteral("DDR");
    case 19: return QStringLiteral("DDR2");
    case 20: return QStringLiteral("DDR2 FB-DIMM");
    case 24: return QStringLiteral("DDR3");
    case 26: return QStringLiteral("DDR4");
    case 27: return QStringLiteral("LPDDR");
    case 28: return QStringLiteral("LPDDR2");
    case 29: return QStringLiteral("LPDDR3");
    case 30: return QStringLiteral("LPDDR4");
    case 34: return QStringLiteral("DDR5");
    case 35: return QStringLiteral("LPDDR5");
    default: return {};
    }
}

InstalledMemory aggregateMemory(const QList<MemoryModule> &modules)
{
    InstalledMemory result;
    if (modules.isEmpty())
        return result;

    result.type = memoryTypeName(modules.first().smbiosMemoryType);
    for (const auto &module : modules) {
        // An incomplete/overflowing total must not be presented as installed RAM.
        if (module.capacityBytes == 0
            || module.capacityBytes > std::numeric_limits<quint64>::max() - result.totalBytes)
            return {};
        result.totalBytes += module.capacityBytes;
        if (memoryTypeName(module.smbiosMemoryType) != result.type)
            result.type.clear();
    }
    return result;
}

QString formatInstalledMemory(const InstalledMemory &memory)
{
    if (memory.totalBytes == 0)
        return QStringLiteral("Unknown RAM");

    // Windows convention: display binary GiB as GB. Preserve fractional capacities.
    constexpr quint64 bytesPerGb = 1024ULL * 1024 * 1024;
    QString amount = QString::number(double(memory.totalBytes) / bytesPerGb, 'f', 2);
    while (amount.endsWith(u'0'))
        amount.chop(1);
    if (amount.endsWith(u'.'))
        amount.chop(1);
    return amount + QStringLiteral(" GB")
           + (memory.type.isEmpty() ? QString() : u' ' + memory.type);
}

QString selectGraphicsAdapter(const QList<GraphicsAdapter> &adapters,
                              const QString &primaryDeviceId)
{
    QStringList names;
    QStringList primaryNames;
    for (const auto &adapter : adapters) {
        const QString name = adapter.name.trimmed();
        if (name.isEmpty())
            continue;
        names.append(name);
        if (!primaryDeviceId.isEmpty()
            && adapter.deviceId.compare(primaryDeviceId, Qt::CaseInsensitive) == 0)
            primaryNames.append(name);
    }
    // Sort independently of WMI enumeration order, even when no primary match exists.
    QStringList &candidates = primaryNames.isEmpty() ? names : primaryNames;
    candidates.sort();
    return candidates.isEmpty() ? QStringLiteral("Unknown GPU") : candidates.first();
}

#ifdef Q_OS_WIN
namespace {
using Microsoft::WRL::ComPtr;

class ComApartment
{
public:
    ComApartment() : result(CoInitializeEx(nullptr, COINIT_MULTITHREADED)) {}
    ~ComApartment() { if (SUCCEEDED(result)) CoUninitialize(); }
    HRESULT result;
};

class Bstr
{
public:
    explicit Bstr(const wchar_t *text) : value(SysAllocString(text)) {}
    ~Bstr() { SysFreeString(value); }
    BSTR value;
};

class Variant
{
public:
    Variant() { VariantInit(&value); }
    ~Variant() { VariantClear(&value); }
    VARIANT value;
};

void logFailure(const char *operation, HRESULT result)
{
    qWarning().noquote() << "Hardware detection:" << operation
                         << "failed (HRESULT 0x" + QString::number(quint32(result), 16) + ")";
}

QString stringProperty(IWbemClassObject *object, const wchar_t *name)
{
    Variant property;
    if (FAILED(object->Get(name, 0, &property.value, nullptr, nullptr))
        || property.value.vt != VT_BSTR || !property.value.bstrVal)
        return {};
    return QString::fromWCharArray(property.value.bstrVal,
                                  int(SysStringLen(property.value.bstrVal))).trimmed();
}

quint64 unsignedProperty(IWbemClassObject *object, const wchar_t *name)
{
    Variant property;
    if (FAILED(object->Get(name, 0, &property.value, nullptr, nullptr)))
        return 0;
    // WMI returns CIM_UINT64 Capacity as a decimal BSTR.
    if (property.value.vt == VT_BSTR && property.value.bstrVal) {
        bool ok = false;
        const auto result = QString::fromWCharArray(property.value.bstrVal).toULongLong(&ok);
        return ok ? result : 0;
    }
    switch (property.value.vt) {
    case VT_UI1: return property.value.bVal;
    case VT_UI2: return property.value.uiVal;
    case VT_UI4: return property.value.ulVal;
    case VT_UI8: return property.value.ullVal;
    case VT_I2: return property.value.iVal >= 0 ? quint64(property.value.iVal) : 0;
    case VT_I4: return property.value.lVal >= 0 ? quint64(property.value.lVal) : 0;
    default: return 0;
    }
}

template<typename Consumer>
bool queryRows(IWbemServices *services, const wchar_t *query, Consumer consume)
{
    Bstr language(L"WQL");
    Bstr statement(query);
    if (!language.value || !statement.value)
        return false;
    ComPtr<IEnumWbemClassObject> rows;
    HRESULT hr = services->ExecQuery(language.value, statement.value,
                                    WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                    nullptr, rows.GetAddressOf());
    if (FAILED(hr)) {
        logFailure("WMI query", hr);
        return false;
    }

    QElapsedTimer deadline;
    deadline.start();
    while (deadline.elapsed() < 5000) {
        ComPtr<IWbemClassObject> row;
        ULONG count = 0;
        hr = rows->Next(250, 1, row.GetAddressOf(), &count);
        if (FAILED(hr)) {
            logFailure("WMI enumeration", hr);
            return false;
        }
        if (count == 1)
            consume(row.Get());
        if (hr == WBEM_S_FALSE)
            return true;
    }
    qWarning() << "Hardware detection: WMI enumeration timed out";
    return false;
}

QString primaryDisplayDeviceId()
{
    DISPLAY_DEVICEW device{};
    device.cb = sizeof(device);
    for (DWORD index = 0; EnumDisplayDevicesW(nullptr, index, &device, 0); ++index) {
        if ((device.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE)
            && (device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
            && !(device.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
            return QString::fromWCharArray(device.DeviceID).trimmed();
        device = {};
        device.cb = sizeof(device);
    }
    return {};
}
} // namespace
#endif

HardwareInfo queryHardwareInfo()
{
    HardwareInfo info;
#ifdef Q_OS_WIN
    ComApartment apartment;
    if (FAILED(apartment.result)) {
        logFailure("COM initialization", apartment.result);
        return info;
    }
    HRESULT hr = CoInitializeSecurity(nullptr, -1, nullptr, nullptr,
                                      RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
                                      nullptr, EOAC_NONE, nullptr);
    // Security is process-wide; Qt/another COM client may have initialized it already.
    if (FAILED(hr) && hr != RPC_E_TOO_LATE) {
        logFailure("COM security", hr);
        return info;
    }

    ComPtr<IWbemLocator> locator;
    hr = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IWbemLocator, reinterpret_cast<void **>(locator.GetAddressOf()));
    if (FAILED(hr)) {
        logFailure("WMI locator", hr);
        return info;
    }
    Bstr namespaceName(L"ROOT\\CIMV2");
    if (!namespaceName.value)
        return info;
    ComPtr<IWbemServices> services;
    hr = locator->ConnectServer(namespaceName.value, nullptr, nullptr, nullptr,
                                WBEM_FLAG_CONNECT_USE_MAX_WAIT, nullptr, nullptr,
                                services.GetAddressOf());
    if (FAILED(hr)) {
        logFailure("WMI connection", hr);
        return info;
    }
    hr = CoSetProxyBlanket(services.Get(), RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr,
                           RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
                           nullptr, EOAC_NONE);
    if (FAILED(hr)) {
        logFailure("WMI proxy security", hr);
        return info;
    }

    QStringList cpuNames;
    if (queryRows(services.Get(), L"SELECT Name FROM Win32_Processor",
                  [&cpuNames](IWbemClassObject *row) {
                      const auto name = stringProperty(row, L"Name");
                      if (!name.isEmpty()) cpuNames.append(name);
                  })) {
        cpuNames.removeDuplicates();
        cpuNames.sort();
        if (!cpuNames.isEmpty()) info.cpuName = cpuNames.join(QStringLiteral(" / "));
    }

    QList<GraphicsAdapter> adapters;
    if (queryRows(services.Get(), L"SELECT Name, PNPDeviceID FROM Win32_VideoController",
                  [&adapters](IWbemClassObject *row) {
                      adapters.append({stringProperty(row, L"Name"),
                                       stringProperty(row, L"PNPDeviceID")});
                  })) {
        for (const auto &adapter : std::as_const(adapters))
            if (!adapter.name.isEmpty()) info.gpuNames.append(adapter.name);
        info.gpuNames.removeDuplicates();
        info.gpuNames.sort();
        const QString primaryId = primaryDisplayDeviceId();
        info.gpuName = selectGraphicsAdapter(adapters, primaryId);
        QStringList matchingIds;
        for (const auto &adapter : std::as_const(adapters)) {
            if (adapter.name == info.gpuName && !adapter.deviceId.isEmpty()) {
                matchingIds.append(adapter.deviceId);
                if (adapter.deviceId.compare(primaryId, Qt::CaseInsensitive) == 0)
                    info.gpuDeviceId = adapter.deviceId;
            }
        }
        matchingIds.sort();
        if (info.gpuDeviceId.isEmpty() && !matchingIds.isEmpty())
            info.gpuDeviceId = matchingIds.first();
    }

    QList<MemoryModule> modules;
    if (queryRows(services.Get(), L"SELECT Capacity, SMBIOSMemoryType FROM Win32_PhysicalMemory",
                  [&modules](IWbemClassObject *row) {
                      const quint64 type = unsignedProperty(row, L"SMBIOSMemoryType");
                      modules.append({unsignedProperty(row, L"Capacity"),
                                      type <= std::numeric_limits<quint16>::max()
                                          ? quint16(type) : quint16(0)});
                  }))
        info.memory = aggregateMemory(modules);
#endif
    return info;
}

HardwareInfoProvider::HardwareInfoProvider(QObject *parent) : QObject(parent)
{
    m_worker.reset(QThread::create([this] {
        const HardwareInfo info = queryHardwareInfo();
        QMetaObject::invokeMethod(this, [this, info] {
            m_info = info;
            emit hardwareInfoChanged();
        }, Qt::QueuedConnection);
    }));
    m_worker->start();
}

HardwareInfoProvider::~HardwareInfoProvider()
{
    // Keep the receiver alive until the one-shot COM worker has released its resources.
    m_worker->wait();
}
