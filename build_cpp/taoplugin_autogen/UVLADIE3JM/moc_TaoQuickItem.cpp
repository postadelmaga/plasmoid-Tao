/****************************************************************************
** Meta object code from reading C++ file 'TaoQuickItem.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../tao-widget/src/TaoQuickItem.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TaoQuickItem.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
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
struct qt_meta_tag_ZN12TaoQuickItemE_t {};
} // unnamed namespace

template <> constexpr inline auto TaoQuickItem::qt_create_metaobjectdata<qt_meta_tag_ZN12TaoQuickItemE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "TaoQuickItem",
        "QML.Element",
        "auto",
        "particleCountChanged",
        "",
        "rotationSpeedChanged",
        "clockwiseChanged",
        "showClockChanged",
        "lowCpuModeChanged",
        "updateAnimation",
        "particleCount",
        "rotationSpeed",
        "clockwise",
        "showClock",
        "lowCpuMode"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'particleCountChanged'
        QtMocHelpers::SignalData<void()>(3, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'rotationSpeedChanged'
        QtMocHelpers::SignalData<void()>(5, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'clockwiseChanged'
        QtMocHelpers::SignalData<void()>(6, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'showClockChanged'
        QtMocHelpers::SignalData<void()>(7, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'lowCpuModeChanged'
        QtMocHelpers::SignalData<void()>(8, 4, QMC::AccessPublic, QMetaType::Void),
        // Slot 'updateAnimation'
        QtMocHelpers::SlotData<void()>(9, 4, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'particleCount'
        QtMocHelpers::PropertyData<int>(10, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'rotationSpeed'
        QtMocHelpers::PropertyData<float>(11, QMetaType::Float, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'clockwise'
        QtMocHelpers::PropertyData<bool>(12, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
        // property 'showClock'
        QtMocHelpers::PropertyData<bool>(13, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'lowCpuMode'
        QtMocHelpers::PropertyData<bool>(14, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
    };
    QtMocHelpers::UintData qt_enums {
    };
    QtMocHelpers::UintData qt_constructors {};
    QtMocHelpers::ClassInfos qt_classinfo({
            {    1,    2 },
    });
    return QtMocHelpers::metaObjectData<TaoQuickItem, void>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums, qt_constructors, qt_classinfo);
}
Q_CONSTINIT const QMetaObject TaoQuickItem::staticMetaObject = { {
    QMetaObject::SuperData::link<QQuickPaintedItem::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12TaoQuickItemE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12TaoQuickItemE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN12TaoQuickItemE_t>.metaTypes,
    nullptr
} };

void TaoQuickItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TaoQuickItem *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->particleCountChanged(); break;
        case 1: _t->rotationSpeedChanged(); break;
        case 2: _t->clockwiseChanged(); break;
        case 3: _t->showClockChanged(); break;
        case 4: _t->lowCpuModeChanged(); break;
        case 5: _t->updateAnimation(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TaoQuickItem::*)()>(_a, &TaoQuickItem::particleCountChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQuickItem::*)()>(_a, &TaoQuickItem::rotationSpeedChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQuickItem::*)()>(_a, &TaoQuickItem::clockwiseChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQuickItem::*)()>(_a, &TaoQuickItem::showClockChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQuickItem::*)()>(_a, &TaoQuickItem::lowCpuModeChanged, 4))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<int*>(_v) = _t->particleCount(); break;
        case 1: *reinterpret_cast<float*>(_v) = _t->rotationSpeed(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->clockwise(); break;
        case 3: *reinterpret_cast<bool*>(_v) = _t->showClock(); break;
        case 4: *reinterpret_cast<bool*>(_v) = _t->lowCpuMode(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setParticleCount(*reinterpret_cast<int*>(_v)); break;
        case 1: _t->setRotationSpeed(*reinterpret_cast<float*>(_v)); break;
        case 2: _t->setClockwise(*reinterpret_cast<bool*>(_v)); break;
        case 3: _t->setShowClock(*reinterpret_cast<bool*>(_v)); break;
        case 4: _t->setLowCpuMode(*reinterpret_cast<bool*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *TaoQuickItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TaoQuickItem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN12TaoQuickItemE_t>.strings))
        return static_cast<void*>(this);
    return QQuickPaintedItem::qt_metacast(_clname);
}

int TaoQuickItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickPaintedItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 6;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void TaoQuickItem::particleCountChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void TaoQuickItem::rotationSpeedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void TaoQuickItem::clockwiseChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void TaoQuickItem::showClockChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void TaoQuickItem::lowCpuModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
