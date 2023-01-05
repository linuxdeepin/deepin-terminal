// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_STUB_DEFINES_H
#define UT_STUB_DEFINES_H

#include "../stub.h"

#include <QDBusMessage>
#include <QDBusConnection>
#include <QDebug>
#include <QTranslator>

class ut_stub_defines
{
public:
    ut_stub_defines();
};

/**
 * 本文件为打桩函数的相关整理，通过RESULT来判断 函数是否被触发
 * ut_CLASS_function                如：ut_QDBusMessage_call            函数:打桩函数
 * ut_CLASS_function_hasRuned       如：ut_QDBusMessage_call_hasRuned   变量:函数是否触发
 * UT_STUB_CLASS_FUNCTION_CREATE    如：UT_STUB_QDBUS_CALL_CREATE       宏:实例化打桩函数
 * UT_STUB_CLASS_FUNCTION_APPEND    如：UT_STUB_QDBUS_CALL_APPEND       宏:只添加stub.set部分
 * UT_STUB_CLASS_FUNCTION_RESULT    如：UT_STUB_QDBUS_CALL_RESULT       宏:函数是否被触发
 * UT_STUB_CLASS_FUNCTION_PREPARE   如：UT_STUB_QDBUS_CALL_PREPARE      宏:初始化hasRuned变量
 */

/******** Add by ut003135 Begin***************/
static  bool ut_QDBusMessage_call_hasRuned = false;
static  QDBusMessage ut_QDBusMessage_call(const QDBusMessage &, QDBus::CallMode,
                             int){
    ut_QDBusMessage_call_hasRuned = true;
    return QDBusMessage();
}
#define UT_STUB_QDBUS_CALL_APPEND \
    stub.set((QDBusMessage (QDBusConnection::*)(QDBusMessage, QDBus::CallMode,int) const)ADDR(QDBusConnection, call),\
            ut_QDBusMessage_call);\
    ut_QDBusMessage_call_hasRuned = false;

#define UT_STUB_QDBUS_CALL_CREATE\
    Stub stub;\
    UT_STUB_QDBUS_CALL_APPEND

#define UT_STUB_QDBUS_CALL_RESULT ut_QDBusMessage_call_hasRuned
#define UT_STUB_QDBUS_CALL_PREPARE ut_QDBusMessage_call_hasRuned = false;
/******** Add by ut003135 End***************/

/******** Add by ut003135 Begin***************/
static  bool ut_QDBusConnection_connect_hasRuned = false;
static  bool ut_QDBusConnection_connect(const QString &, const QString &, const QString & , const QString &, QObject *, const char *){
    ut_QDBusConnection_connect_hasRuned = true;
    return true;
}
#define UT_STUB_QDBUS_CONNECT_APPEND \
    stub.set((bool (QDBusConnection::*)(const QString &, const QString &, const QString & , const QString &, QObject *, const char *) )ADDR(QDBusConnection, connect),\
            ut_QDBusConnection_connect);\
    ut_QDBusConnection_connect_hasRuned = false;

#define UT_STUB_QDBUS_CONNECT_CREATE\
    Stub stub;\
    UT_STUB_QDBUS_CONNECT_APPEND

#define UT_STUB_QDBUS_CONNECT_RESULT ut_QDBusConnection_connect_hasRuned
#define UT_STUB_QDBUS_CONNECT_PREPARE ut_QDBusConnection_connect_hasRuned = false;
/******** Add by ut003135 End***************/


/******** Add by ut003135 Begin***************/
static  bool ut_QWidget_setFocus_hasRuned = false;
static  void ut_QWidget_setFocus(Qt::FocusReason){
    ut_QWidget_setFocus_hasRuned = true;
}
#define UT_STUB_QWIDGET_SETFOCUS_APPEND\
    stub.set((void (QWidget::*)(Qt::FocusReason))ADDR(QWidget, setFocus), ut_QWidget_setFocus);\
    ut_QWidget_setFocus_hasRuned = false;\

#define UT_STUB_QWIDGET_SETFOCUS_CREATE\
    Stub stub;\
    UT_STUB_QWIDGET_SETFOCUS_APPEND

#define UT_STUB_QWIDGET_SETFOCUS_RESULT ut_QWidget_setFocus_hasRuned
#define UT_STUB_QWIDGET_SETFOCUS_PREPARE ut_QWidget_setFocus_hasRuned = false;
/******** Add by ut003135 End***************/

/******** Add by ut003135 Begin***************/
static  bool ut_QWidget_update_hasRuned = false;
static  void ut_QWidget_update(){
    ut_QWidget_update_hasRuned = true;
}
#define UT_STUB_QWIDGET_UPDATES_APPEND\
    stub.set((void (QWidget::*)())ADDR(QWidget, update), ut_QWidget_update);\
    ut_QWidget_update_hasRuned = false;\

#define UT_STUB_QWIDGET_UPDATES_CREATE\
    Stub stub;\
    UT_STUB_QWIDGET_UPDATES_APPEND

#define UT_STUB_QWIDGET_UPDATES_RESULT ut_QWidget_update_hasRuned
#define UT_STUB_QWIDGET_UPDATES_PREPARE ut_QWidget_update_hasRuned = false;
/******** Add by ut003135 End***************/


/******** Add by ut003135 Begin***************/
static  bool ut_QWidget_hasFocus_hasRuned = false;
static  bool ut_QWidget_hasFocus(){
    ut_QWidget_hasFocus_hasRuned = true;
    return true;
}
#define UT_STUB_QWIDGET_HASFOCUS_APPEND\
    stub.set((bool (QWidget::*)())ADDR(QWidget, hasFocus), ut_QWidget_hasFocus);\
    ut_QWidget_hasFocus_hasRuned = false;\

#define UT_STUB_QWIDGET_HASFOCUS_CREATE\
    Stub stub;\
    UT_STUB_QWIDGET_HASFOCUS_APPEND

#define UT_STUB_QWIDGET_HASFOCUS_RESULT ut_QWidget_hasFocus_hasRuned
#define UT_STUB_QWIDGET_HASFOCUS_PREPARE ut_QWidget_hasFocus_hasRuned = false;
/******** Add by ut003135 End***************/


/******** Add by ut003135 Begin***************/
static  bool ut_QWidget_isVisible_hasRuned = false;
static  bool ut_QWidget_isVisible(){
    ut_QWidget_isVisible_hasRuned = true;
    return true;
}

#define UT_STUB_QWIDGET_ISVISIBLE_APPEND\
    stub.set((bool (QWidget::*)() const)ADDR(QWidget, isVisible), ut_QWidget_isVisible);\
    ut_QWidget_isVisible_hasRuned = false;\

#define UT_STUB_QWIDGET_ISVISIBLE_CREATE\
    Stub stub;\
    UT_STUB_QWIDGET_ISVISIBLE_APPEND

#define UT_STUB_QWIDGET_ISVISIBLE_RESULT ut_QWidget_isVisible_hasRuned
#define UT_STUB_QWIDGET_ISVISIBLE_PREPARE ut_QWidget_isVisible_hasRuned = false;
/******** Add by ut003135 End***************/

/******** Add by ut003135 Begin***************/
static  bool ut_QProcess_startDetached_hasRuned = false;
static  bool ut_QProcess_startDetached(qint64 *){
    ut_QProcess_startDetached_hasRuned = true;
    return true;
}

#define UT_STUB_QPROCESS_STARTDETACHED_APPEND\
    stub.set((bool (QProcess::*)(qint64 *))ADDR(QProcess, startDetached), ut_QProcess_startDetached);\
    ut_QProcess_startDetached_hasRuned = false;\

#define UT_STUB_QPROCESS_STARTDETACHED_CREATE\
    Stub stub;\
    UT_STUB_QPROCESS_STARTDETACHED_APPEND

#define UT_STUB_QPROCESS_STARTDETACHED_RESULT ut_QProcess_startDetached_hasRuned
#define UT_STUB_QPROCESS_STARTDETACHED_PREPARE ut_QProcess_startDetached_hasRuned = false;
/******** Add by ut003135 End***************/


/******** Add by ut003135 Begin***************/
static  bool ut_QWidget_setVisible_hasRuned = false;
static  void ut_QWidget_setVisible(void *, bool){
    ut_QWidget_setVisible_hasRuned = true;
}
#define UT_STUB_QWIDGET_SETVISIBLE_APPEND\
    typedef void (*ut_QWidget_setVisible_ptr)(QWidget*, bool); \
    ut_QWidget_setVisible_ptr ptr = (ut_QWidget_setVisible_ptr)(&QWidget::setVisible);   \
    stub.set(ptr, ut_QWidget_setVisible); \
    ut_QWidget_setVisible_hasRuned = false;\

#define UT_STUB_QWIDGET_SETVISIBLE_CREATE\
    Stub stub;\
    UT_STUB_QWIDGET_SETVISIBLE_APPEND

#define UT_STUB_QWIDGET_SETVISIBLE_RESULT ut_QWidget_setVisible_hasRuned
#define UT_STUB_QWIDGET_SETVISIBLE_PREPARE ut_QWidget_setVisible_hasRuned = false;
/******** Add by ut003135 End***************/

/******** Add by ut003135 Begin***************/
static  bool ut_QDialog_open_hasRuned = false;
static  void ut_QDialog_open(void *){
    ut_QDialog_open_hasRuned = true;
}
#define UT_STUB_QDIALOG_OPEN_APPEND\
    typedef void (*ut_QDialog_open_ptr)(QDialog*); \
    ut_QDialog_open_ptr ptr = (ut_QDialog_open_ptr)(&QDialog::open);   \
    stub.set(ptr, ut_QDialog_open); \
    ut_QDialog_open_hasRuned = false;\

#define UT_STUB_QDIALOG_OPEN_CREATE\
    Stub stub;\
    UT_STUB_QDIALOG_OPEN_APPEND

#define UT_STUB_QDIALOG_OPEN_RESULT ut_QDialog_open_hasRuned
#define UT_STUB_QDIALOG_OPEN_PREPARE ut_QDialog_open_hasRuned = false;
/******** Add by ut003135 End***************/



/******** Add by ut003135 Begin***************/
static  bool ut_QTranslator_translate_hasRuned = false;
static  QString ut_QTranslator_translate(void *, const char *, const char *, const char *, int){
    ut_QTranslator_translate_hasRuned = true;
    return QString();
}

typedef QString (*ut_QTranslator_translate_ptr)(QTranslator*, const char *, const char *, const char *, int) ;

#define UT_STUB_QTRANSLATE_TRANSLATE_APPEND\
    ut_QTranslator_translate_ptr ptr = (ut_QTranslator_translate_ptr)(&QTranslator::translate);   \
    stub.set(ptr, ut_QTranslator_translate); \
    ut_QTranslator_translate_hasRuned = false;\

#define UT_STUB_QTRANSLATE_TRANSLATE_CREATE\
    Stub stub;\
    UT_STUB_QTRANSLATE_TRANSLATE_APPEND

#define UT_STUB_QTRANSLATE_TRANSLATE_RESULT ut_QTranslator_translate_hasRuned
#define UT_STUB_QTRANSLATE_TRANSLATE_PREPARE ut_QTranslator_translate_hasRuned = false;
/******** Add by ut003135 End***************/


/******** Add by ut003135 Begin***************/
static  bool ut_QWidget_show_hasRuned = false;
static  void ut_QWidget_show(){
    ut_QWidget_show_hasRuned = true;
}

#define UT_STUB_QWIDGET_SHOW_APPEND\
    stub.set((void (QWidget::*)())ADDR(QWidget, show), ut_QWidget_show); \
    ut_QWidget_show_hasRuned = false;\

#define UT_STUB_QWIDGET_SHOW_CREATE\
    Stub stub;\
    UT_STUB_QWIDGET_SHOW_APPEND

#define UT_STUB_QWIDGET_SHOW_RESULT ut_QWidget_show_hasRuned
#define UT_STUB_QWIDGET_SHOW_PREPARE ut_QWidget_show_hasRuned = false;
/******** Add by ut003135 End***************/


/******** Add by ut003135 Begin***************/
static  bool ut_QUrl_isValid_hasRuned = false;
static  bool ut_QUrl_isValid(){
    ut_QUrl_isValid_hasRuned = true;
    return false;
}

#define UT_STUB_QURL_ISVALID_APPEND\
    stub.set((bool (QUrl::*)() const)ADDR(QUrl, isValid), ut_QUrl_isValid); \
    ut_QUrl_isValid_hasRuned = false;\

#define UT_STUB_QURL_ISVALID_CREATE\
    Stub stub;\
    UT_STUB_QURL_ISVALID_APPEND

#define UT_STUB_QURL_ISVALID_RESULT ut_QUrl_isValid_hasRuned
#define UT_STUB_QURL_ISVALID_PREPARE ut_QUrl_isValid_hasRuned = false;
/******** Add by ut003135 End***************/


/******** Add by ut003135 Begin***************/
static  bool ut_QApplication_sendEvent_hasRuned = false;
static  bool ut_QApplication_sendEvent(QObject *, QEvent *){
    ut_QApplication_sendEvent_hasRuned = true;
    return false;
}

#define UT_STUB_QAPPLICATION_SENDEVENT_APPEND\
    stub.set(ADDR(QApplication, sendEvent), ut_QApplication_sendEvent); \
    ut_QApplication_sendEvent_hasRuned = false;\

#define UT_STUB_QAPPLICATION_SENDEVENT_CREATE\
    Stub stub;\
    UT_STUB_QAPPLICATION_SENDEVENT_APPEND

#define UT_STUB_QAPPLICATION_SENDEVENT_RESULT ut_QApplication_sendEvent_hasRuned
#define UT_STUB_QAPPLICATION_SENDEVENT_PREPARE ut_QApplication_sendEvent_hasRuned = false;
/******** Add by ut003135 End***************/

/******** Add by ut003135 Begin***************/
static  bool ut_DSettingsOption_value_hasRuned = false;
static  QVariant ut_DSettingsOption_value(){
    ut_DSettingsOption_value_hasRuned = true;
    return QVariant();
}

#define UT_STUB_DSETTINGSOPTION_VALUE_APPEND\
    stub.set(ADDR(DSettingsOption, value), ut_DSettingsOption_value); \
    ut_DSettingsOption_value_hasRuned = false;\

#define UT_STUB_DSETTINGSOPTION_VALUE_CREATE\
    Stub stub;\
    UT_STUB_DSETTINGSOPTION_VALUE_APPEND

#define UT_STUB_DSETTINGSOPTION_VALUE_RESULT ut_DSettingsOption_value_hasRuned
#define UT_STUB_DSETTINGSOPTION_VALUE_PREPARE ut_DSettingsOption_value_hasRuned = false;
/******** Add by ut003135 End***************/


/******** Add by ut003135 Begin***************/
static  bool ut_QDir_exists_hasRuned = false;
static  bool ut_QDir_exists(){
    ut_QDir_exists_hasRuned = true;
    return true;
}

#define UT_STUB_QDIR_EXISTS_APPEND\
    stub.set((bool (QDir::*)()const)ADDR(QDir, exists), ut_QDir_exists); \
    ut_QDir_exists_hasRuned = false;\

#define UT_STUB_QDIR_EXISTS_CREATE\
    Stub stub;\
    UT_STUB_QDIR_EXISTS_APPEND

#define UT_STUB_QDIR_EXISTS_RESULT ut_QDir_exists_hasRuned
#define UT_STUB_QDIR_EXISTS_PREPARE ut_QDir_exists_hasRuned = false;
/******** Add by ut003135 End***************/

/******** Add by ut003135 Begin***************/
static  bool ut_QFile_remove_hasRuned = false;
static  bool ut_QFile_remove(){
    ut_QFile_remove_hasRuned = true;
    return true;
}

#define UT_STUB_QFILE_REMOVE_APPEND\
    stub.set((bool (QFile::*)())ADDR(QFile, remove), ut_QFile_remove); \
    ut_QFile_remove_hasRuned = false;\

#define UT_STUB_QFILE_REMOVE_CREATE\
    Stub stub;\
    UT_STUB_QFILE_REMOVE_APPEND

#define UT_STUB_QFILE_REMOVE_RESULT ut_QFile_remove_hasRuned
#define UT_STUB_QFILE_REMOVE_PREPARE ut_QFile_remove_hasRuned = false;
/******** Add by ut003135 End***************/

/******** Add by ut003135 Begin***************/
static  bool ut_QObject_findChild_QWidget_hasRuned = false;
static QWidget *ut_QObject_findChild_QWidget(void* , const QString &, Qt::FindChildOptions)
{
    ut_QObject_findChild_QWidget_hasRuned = true;
    return nullptr;
}


#define UT_STUB_QOBJECT_FINDCHILD_QWIDGET_APPEND\
    stub.set((QWidget *(QObject::*)(const QString &, Qt::FindChildOptions) const)ADDR(QObject, findChild), ut_QObject_findChild_QWidget);     \
    ut_QObject_findChild_QWidget_hasRuned = false;\

#define UT_STUB_QOBJECT_FINDCHILD_QWIDGET_CREATE\
    Stub stub;\
    UT_STUB_QOBJECT_FINDCHILD_QWIDGET_APPEND

#define UT_STUB_QOBJECT_FINDCHILD_QWIDGET_RESULT ut_QObject_findChild_QWidget_hasRuned
#define UT_STUB_QOBJECT_FINDCHILD_QWIDGET_PREPARE ut_QObject_findChild_QWidget_hasRuned = false;
/******** Add by ut003135 End***************/

#define DELETE_PTR(ptr) \
    if(ptr) {\
    delete ptr;\
    ptr = nullptr;\
    }

#define DELETE_PTR_LATER(ptr) \
    if(ptr) {\
    ptr->deleteLater();\
    ptr = nullptr;\
    }

#endif // UT_STUB_DEFINES_H
