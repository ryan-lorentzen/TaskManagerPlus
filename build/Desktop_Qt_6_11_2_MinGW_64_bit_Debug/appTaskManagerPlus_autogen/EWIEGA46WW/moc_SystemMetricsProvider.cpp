/****************************************************************************
** Meta object code from reading C++ file 'SystemMetricsProvider.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../SystemMetricsProvider.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SystemMetricsProvider.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN21SystemMetricsProviderE_t {};
} // unnamed namespace

template <> constexpr inline auto SystemMetricsProvider::qt_create_metaobjectdata<qt_meta_tag_ZN21SystemMetricsProviderE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SystemMetricsProvider",
        "snapshotChanged",
        "",
        "temperatureText",
        "QVariant",
        "celsius",
        "fahrenheit",
        "cpuUsagePercent",
        "gpuUsagePercent",
        "cpuTemperatureC",
        "gpuTemperatureC",
        "memoryUsedText",
        "memorySummaryText",
        "memoryUsedRatio",
        "cpuHistory",
        "QVariantList",
        "gpuHistory",
        "sampleTimeMs"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'snapshotChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'temperatureText'
        QtMocHelpers::MethodData<QString(const QVariant &, bool) const>(3, 2, QMC::AccessPublic, QMetaType::QString, {{
            { 0x80000000 | 4, 5 }, { QMetaType::Bool, 6 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'cpuUsagePercent'
        QtMocHelpers::PropertyData<QVariant>(7, 0x80000000 | 4, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'gpuUsagePercent'
        QtMocHelpers::PropertyData<QVariant>(8, 0x80000000 | 4, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'cpuTemperatureC'
        QtMocHelpers::PropertyData<QVariant>(9, 0x80000000 | 4, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'gpuTemperatureC'
        QtMocHelpers::PropertyData<QVariant>(10, 0x80000000 | 4, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'memoryUsedText'
        QtMocHelpers::PropertyData<QString>(11, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'memorySummaryText'
        QtMocHelpers::PropertyData<QString>(12, QMetaType::QString, QMC::DefaultPropertyFlags, 0),
        // property 'memoryUsedRatio'
        QtMocHelpers::PropertyData<double>(13, QMetaType::Double, QMC::DefaultPropertyFlags, 0),
        // property 'cpuHistory'
        QtMocHelpers::PropertyData<QVariantList>(14, 0x80000000 | 15, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'gpuHistory'
        QtMocHelpers::PropertyData<QVariantList>(16, 0x80000000 | 15, QMC::DefaultPropertyFlags | QMC::EnumOrFlag, 0),
        // property 'sampleTimeMs'
        QtMocHelpers::PropertyData<qint64>(17, QMetaType::LongLong, QMC::DefaultPropertyFlags, 0),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SystemMetricsProvider, qt_meta_tag_ZN21SystemMetricsProviderE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SystemMetricsProvider::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN21SystemMetricsProviderE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN21SystemMetricsProviderE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN21SystemMetricsProviderE_t>.metaTypes,
    nullptr
} };

void SystemMetricsProvider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SystemMetricsProvider *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->snapshotChanged(); break;
        case 1: { QString _r = _t->temperatureText((*reinterpret_cast<std::add_pointer_t<QVariant>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<bool>>(_a[2])));
            if (_a[0]) *reinterpret_cast<QString*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SystemMetricsProvider::*)()>(_a, &SystemMetricsProvider::snapshotChanged, 0))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QVariant*>(_v) = _t->cpuUsagePercent(); break;
        case 1: *reinterpret_cast<QVariant*>(_v) = _t->gpuUsagePercent(); break;
        case 2: *reinterpret_cast<QVariant*>(_v) = _t->cpuTemperatureC(); break;
        case 3: *reinterpret_cast<QVariant*>(_v) = _t->gpuTemperatureC(); break;
        case 4: *reinterpret_cast<QString*>(_v) = _t->memoryUsedText(); break;
        case 5: *reinterpret_cast<QString*>(_v) = _t->memorySummaryText(); break;
        case 6: *reinterpret_cast<double*>(_v) = _t->memoryUsedRatio(); break;
        case 7: *reinterpret_cast<QVariantList*>(_v) = _t->cpuHistory(); break;
        case 8: *reinterpret_cast<QVariantList*>(_v) = _t->gpuHistory(); break;
        case 9: *reinterpret_cast<qint64*>(_v) = _t->sampleTimeMs(); break;
        default: break;
        }
    }
}

const QMetaObject *SystemMetricsProvider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SystemMetricsProvider::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN21SystemMetricsProviderE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SystemMetricsProvider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void SystemMetricsProvider::snapshotChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
