/****************************************************************************
** Meta object code from reading C++ file 'TaoQGraphHybrid.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../tao-widget/src/TaoQGraphHybrid.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TaoQGraphHybrid.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN15TaoQGraphHybridE_t {};
} // unnamed namespace

template <> constexpr inline auto TaoQGraphHybrid::qt_create_metaobjectdata<qt_meta_tag_ZN15TaoQGraphHybridE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "TaoQGraphHybrid",
        "QML.Element",
        "auto",
        "particleCountChanged",
        "",
        "rotationSpeedChanged",
        "clockwiseChanged",
        "showClockChanged",
        "lowCpuModeChanged",
        "mousePosChanged",
        "hourHandColorChanged",
        "minuteHandColorChanged",
        "secondHandColorChanged",
        "glowColor1Changed",
        "glowSize1Changed",
        "glowColor2Changed",
        "glowSize2Changed",
        "particleColor1Changed",
        "particleColor2Changed",
        "particleCount",
        "rotationSpeed",
        "clockwise",
        "showClock",
        "lowCpuMode",
        "mousePos",
        "QPointF",
        "hourHandColor",
        "QColor",
        "minuteHandColor",
        "secondHandColor",
        "glowColor1",
        "glowSize1",
        "glowColor2",
        "glowSize2",
        "particleColor1",
        "particleColor2"
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
        // Signal 'mousePosChanged'
        QtMocHelpers::SignalData<void()>(9, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'hourHandColorChanged'
        QtMocHelpers::SignalData<void()>(10, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'minuteHandColorChanged'
        QtMocHelpers::SignalData<void()>(11, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'secondHandColorChanged'
        QtMocHelpers::SignalData<void()>(12, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'glowColor1Changed'
        QtMocHelpers::SignalData<void()>(13, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'glowSize1Changed'
        QtMocHelpers::SignalData<void()>(14, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'glowColor2Changed'
        QtMocHelpers::SignalData<void()>(15, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'glowSize2Changed'
        QtMocHelpers::SignalData<void()>(16, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'particleColor1Changed'
        QtMocHelpers::SignalData<void()>(17, 4, QMC::AccessPublic, QMetaType::Void),
        // Signal 'particleColor2Changed'
        QtMocHelpers::SignalData<void()>(18, 4, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'particleCount'
        QtMocHelpers::PropertyData<int>(19, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'rotationSpeed'
        QtMocHelpers::PropertyData<float>(20, QMetaType::Float, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'clockwise'
        QtMocHelpers::PropertyData<bool>(21, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 2),
        // property 'showClock'
        QtMocHelpers::PropertyData<bool>(22, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 3),
        // property 'lowCpuMode'
        QtMocHelpers::PropertyData<bool>(23, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 4),
        // property 'mousePos'
        QtMocHelpers::PropertyData<QPointF>(24, 0x80000000 | 25, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 5),
        // property 'hourHandColor'
        QtMocHelpers::PropertyData<QColor>(26, 0x80000000 | 27, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 6),
        // property 'minuteHandColor'
        QtMocHelpers::PropertyData<QColor>(28, 0x80000000 | 27, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 7),
        // property 'secondHandColor'
        QtMocHelpers::PropertyData<QColor>(29, 0x80000000 | 27, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 8),
        // property 'glowColor1'
        QtMocHelpers::PropertyData<QColor>(30, 0x80000000 | 27, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 9),
        // property 'glowSize1'
        QtMocHelpers::PropertyData<double>(31, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 10),
        // property 'glowColor2'
        QtMocHelpers::PropertyData<QColor>(32, 0x80000000 | 27, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 11),
        // property 'glowSize2'
        QtMocHelpers::PropertyData<double>(33, QMetaType::Double, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 12),
        // property 'particleColor1'
        QtMocHelpers::PropertyData<QColor>(34, 0x80000000 | 27, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 13),
        // property 'particleColor2'
        QtMocHelpers::PropertyData<QColor>(35, 0x80000000 | 27, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet, 14),
    };
    QtMocHelpers::UintData qt_enums {
    };
    QtMocHelpers::UintData qt_constructors {};
    QtMocHelpers::ClassInfos qt_classinfo({
            {    1,    2 },
    });
    return QtMocHelpers::metaObjectData<TaoQGraphHybrid, void>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums, qt_constructors, qt_classinfo);
}
Q_CONSTINIT const QMetaObject TaoQGraphHybrid::staticMetaObject = { {
    QMetaObject::SuperData::link<QQuickItem::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15TaoQGraphHybridE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15TaoQGraphHybridE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN15TaoQGraphHybridE_t>.metaTypes,
    nullptr
} };

void TaoQGraphHybrid::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TaoQGraphHybrid *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->particleCountChanged(); break;
        case 1: _t->rotationSpeedChanged(); break;
        case 2: _t->clockwiseChanged(); break;
        case 3: _t->showClockChanged(); break;
        case 4: _t->lowCpuModeChanged(); break;
        case 5: _t->mousePosChanged(); break;
        case 6: _t->hourHandColorChanged(); break;
        case 7: _t->minuteHandColorChanged(); break;
        case 8: _t->secondHandColorChanged(); break;
        case 9: _t->glowColor1Changed(); break;
        case 10: _t->glowSize1Changed(); break;
        case 11: _t->glowColor2Changed(); break;
        case 12: _t->glowSize2Changed(); break;
        case 13: _t->particleColor1Changed(); break;
        case 14: _t->particleColor2Changed(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::particleCountChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::rotationSpeedChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::clockwiseChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::showClockChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::lowCpuModeChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::mousePosChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::hourHandColorChanged, 6))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::minuteHandColorChanged, 7))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::secondHandColorChanged, 8))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::glowColor1Changed, 9))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::glowSize1Changed, 10))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::glowColor2Changed, 11))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::glowSize2Changed, 12))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::particleColor1Changed, 13))
            return;
        if (QtMocHelpers::indexOfMethod<void (TaoQGraphHybrid::*)()>(_a, &TaoQGraphHybrid::particleColor2Changed, 14))
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
        case 5: *reinterpret_cast<QPointF*>(_v) = _t->mousePos(); break;
        case 6: *reinterpret_cast<QColor*>(_v) = _t->hourHandColor(); break;
        case 7: *reinterpret_cast<QColor*>(_v) = _t->minuteHandColor(); break;
        case 8: *reinterpret_cast<QColor*>(_v) = _t->secondHandColor(); break;
        case 9: *reinterpret_cast<QColor*>(_v) = _t->glowColor1(); break;
        case 10: *reinterpret_cast<double*>(_v) = _t->glowSize1(); break;
        case 11: *reinterpret_cast<QColor*>(_v) = _t->glowColor2(); break;
        case 12: *reinterpret_cast<double*>(_v) = _t->glowSize2(); break;
        case 13: *reinterpret_cast<QColor*>(_v) = _t->particleColor1(); break;
        case 14: *reinterpret_cast<QColor*>(_v) = _t->particleColor2(); break;
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
        case 5: _t->setMousePos(*reinterpret_cast<QPointF*>(_v)); break;
        case 6: _t->setHourHandColor(*reinterpret_cast<QColor*>(_v)); break;
        case 7: _t->setMinuteHandColor(*reinterpret_cast<QColor*>(_v)); break;
        case 8: _t->setSecondHandColor(*reinterpret_cast<QColor*>(_v)); break;
        case 9: _t->setGlowColor1(*reinterpret_cast<QColor*>(_v)); break;
        case 10: _t->setGlowSize1(*reinterpret_cast<double*>(_v)); break;
        case 11: _t->setGlowColor2(*reinterpret_cast<QColor*>(_v)); break;
        case 12: _t->setGlowSize2(*reinterpret_cast<double*>(_v)); break;
        case 13: _t->setParticleColor1(*reinterpret_cast<QColor*>(_v)); break;
        case 14: _t->setParticleColor2(*reinterpret_cast<QColor*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *TaoQGraphHybrid::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TaoQGraphHybrid::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN15TaoQGraphHybridE_t>.strings))
        return static_cast<void*>(this);
    return QQuickItem::qt_metacast(_clname);
}

int TaoQGraphHybrid::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 15;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void TaoQGraphHybrid::particleCountChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void TaoQGraphHybrid::rotationSpeedChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void TaoQGraphHybrid::clockwiseChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void TaoQGraphHybrid::showClockChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void TaoQGraphHybrid::lowCpuModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void TaoQGraphHybrid::mousePosChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void TaoQGraphHybrid::hourHandColorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void TaoQGraphHybrid::minuteHandColorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void TaoQGraphHybrid::secondHandColorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void TaoQGraphHybrid::glowColor1Changed()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void TaoQGraphHybrid::glowSize1Changed()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void TaoQGraphHybrid::glowColor2Changed()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void TaoQGraphHybrid::glowSize2Changed()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void TaoQGraphHybrid::particleColor1Changed()
{
    QMetaObject::activate(this, &staticMetaObject, 13, nullptr);
}

// SIGNAL 14
void TaoQGraphHybrid::particleColor2Changed()
{
    QMetaObject::activate(this, &staticMetaObject, 14, nullptr);
}
QT_WARNING_POP
