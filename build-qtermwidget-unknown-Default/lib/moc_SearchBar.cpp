/****************************************************************************
** Meta object code from reading C++ file 'SearchBar.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.11.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../qtermwidget/lib/SearchBar.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SearchBar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.11.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SearchBar_t {
    QByteArrayData data[9];
    char stringdata0[130];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SearchBar_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SearchBar_t qt_meta_stringdata_SearchBar = {
    {
QT_MOC_LITERAL(0, 0, 9), // "SearchBar"
QT_MOC_LITERAL(1, 10, 21), // "searchCriteriaChanged"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 23), // "highlightMatchesChanged"
QT_MOC_LITERAL(4, 57, 16), // "highlightMatches"
QT_MOC_LITERAL(5, 74, 8), // "findNext"
QT_MOC_LITERAL(6, 83, 12), // "findPrevious"
QT_MOC_LITERAL(7, 96, 12), // "noMatchFound"
QT_MOC_LITERAL(8, 109, 20) // "clearBackgroundColor"

    },
    "SearchBar\0searchCriteriaChanged\0\0"
    "highlightMatchesChanged\0highlightMatches\0"
    "findNext\0findPrevious\0noMatchFound\0"
    "clearBackgroundColor"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SearchBar[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,
       3,    1,   45,    2, 0x06 /* Public */,
       5,    0,   48,    2, 0x06 /* Public */,
       6,    0,   49,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    0,   50,    2, 0x0a /* Public */,
       8,    0,   51,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void SearchBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SearchBar *_t = static_cast<SearchBar *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->searchCriteriaChanged(); break;
        case 1: _t->highlightMatchesChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->findNext(); break;
        case 3: _t->findPrevious(); break;
        case 4: _t->noMatchFound(); break;
        case 5: _t->clearBackgroundColor(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SearchBar::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SearchBar::searchCriteriaChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SearchBar::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SearchBar::highlightMatchesChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SearchBar::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SearchBar::findNext)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (SearchBar::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SearchBar::findPrevious)) {
                *result = 3;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SearchBar::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_SearchBar.data,
      qt_meta_data_SearchBar,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *SearchBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SearchBar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SearchBar.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int SearchBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void SearchBar::searchCriteriaChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SearchBar::highlightMatchesChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SearchBar::findNext()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SearchBar::findPrevious()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
