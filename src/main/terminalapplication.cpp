// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "terminalapplication.h"
#include "mainwindow.h"
#include "service.h"
#include "settings.h"

// qt
#include <QDebug>

#ifdef QT_DEBUG
#include <QTranslator>
#endif

TerminalApplication::TerminalApplication(int &argc, char *argv[]) : DApplication(argc, argv)
{
    Utils::set_Object_Name(this);
    loadTranslator();
    setOrganizationName("deepin");
    setApplicationVersion(VERSION);
    setApplicationName("deepin-terminal");
    setApplicationDisplayName(QObject::tr("Terminal"));
    setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    setProductIcon(QIcon::fromTheme("deepin-terminal"));
    QString appDesc = QObject::tr("Terminal is an advanced terminal emulator with workspace"
                                  ", multiple windows, remote management, quake mode and other features.");
    setApplicationDescription(appDesc);

    /***add by ut001121 zhangmeng 20200617 禁用应用程序自动退出 修复BUG33541***/
    setQuitOnLastWindowClosed(false);

#ifdef QT_DEBUG
    QTranslator translator;
    translator.load(QString("deepin-terminal_%1").arg(QLocale::system().name()));
    installTranslator(&translator);
#endif  // QT_DEBUG
}

TerminalApplication::~TerminalApplication()
{
    Service::releaseInstance();
    Settings::releaseInstance();
}

void TerminalApplication::setStartTime(qint64 time)
{
    m_AppStartTime = time;
}

qint64 TerminalApplication::getStartTime()
{
    return m_AppStartTime;
}

void TerminalApplication::handleQuitAction()
{
    qInfo() << "handleQuitAction";
    activeWindow()->close();
}


bool TerminalApplication::notify(QObject *object, QEvent *event)
{
    //修复bug#110813,参考https://gerrit.uniontech.com/c/dtkwidget/+/94656
    if (event->type() == QEvent::ApplicationFontChange) {
        // ApplicationFontChange 调用 font() 是 ok 的，如果在 fontChanged 中调用在某些版本中会出现 deadlock
        DFontSizeManager::instance()->setFontGenericPixelSize(static_cast<quint16>(DFontSizeManager::fontPixelSize(font())));
    }

    // 针对DTK做的特殊处理,等DTK自己完成后,需要删除
    if (QStringLiteral("Dtk::Widget::DKeySequenceEdit") == object->metaObject()->className()) {
        // 焦点移除,移除edit
        if (QEvent::FocusOut == event->type()) {
            DKeySequenceEdit *edit = static_cast<DKeySequenceEdit *>(object);
            // 包含edit
            if (m_keySequenceList.contains(edit)) {
                m_keySequenceList.removeOne(edit);
                qInfo() << "remove editing when foucs out";
            }
        }
        if (QEvent::KeyPress == event->type()) {
            QKeyEvent *keyevent = static_cast<QKeyEvent *>(event);
            // 获取DKeySequenceEdit
            DKeySequenceEdit *edit = static_cast<DKeySequenceEdit *>(object);
            // 在dtk的自定义快捷键输入框上按Enter
            if (Qt::Key_Enter == keyevent->key()
                    || Qt::Key_Return == keyevent->key()
                    || Qt::Key_Space == keyevent->key()
               ) {
                // 当快捷键输入框内容不为空
                // 设置里的快捷键输入框
                if (!edit->keySequence().isEmpty()
                        && ("OptionShortcutEdit" == object->objectName() || ("CustomShortCutLineEdit" == object->objectName() && Qt::Key_Space == keyevent->key()))
                        && !m_keySequenceList.contains(edit)) {
                    // 找到其中的QLabel
                    QList<Dtk::Widget::DLabel *> childern = edit->findChildren<Dtk::Widget::DLabel *>();
                    // 发送QMouseEvent
                    QMouseEvent mouseEvent(QEvent::MouseButtonPress, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
                    QApplication::sendEvent(childern[0], &mouseEvent);
                    // 记录当前KeySequence已经进入编辑状态
                    qInfo() << "KeySequence in Editing";
                    m_keySequenceList.append(edit);
                    return true;
                }
            }
            // 不是Enter和space操作，或者没进入记录流程，就删除
            if (m_keySequenceList.contains(edit)) {
                // 其他情况的按键，移除edit
                m_keySequenceList.removeOne(edit);
                qInfo() << "remove editing when others";
            }
        }
    }

    // ALT+M = 右键
    if (QEvent::KeyPress == event->type()) {
        QKeyEvent *keyevent = static_cast<QKeyEvent *>(event);
        /***add begin by ut001121 zhangmeng 20200801 截获DPushButton控件回车按键事件并模拟空格键点击事件,用以解决回车键不响应的问题***/
        // 回车键
        // 恢复默认 添健按钮
        if ((QStringLiteral("QPushButton") == object->metaObject()->className()
                // 远程和自定义列表的返回按钮，编辑按钮
                || QStringLiteral("IconButton") == object->metaObject()->className()
                // 搜索框的上下搜索
                || QStringLiteral("Dtk::Widget::DIconButton") == object->metaObject()->className()
                // 设置里面的单选框
                || QStringLiteral("QCheckBox") == object->metaObject()->className()
                // 设置字体组合框
                || QStringLiteral("QComboBox") == object->metaObject()->className()
                //1050上DComboBox和QComBobox做了区分，相对的DSettingsDialog的ComboBox改用了DComboBox
                || QStringLiteral("Dtk::Widget::DComboBox") == object->metaObject()->className()
                // 设置窗口组合框
                || QStringLiteral("ComboBox") == object->metaObject()->className())
                && (Qt::Key_Return == keyevent->key() || Qt::Key_Enter == keyevent->key())) {
            // 模拟空格键按下事件
            pressSpace(object);
            return true;
        }
        /***add end by ut001121***/
        // 左键
        // 远程和自定义列表的返回按钮 Key_Left
        if ((QStringLiteral("CustomRebackButton") == object->objectName()
                || QStringLiteral("RemoteSearchRebackButton") == object->objectName()
                || QStringLiteral("RemoteGroupRebackButton") == object->objectName())
                && Qt::Key_Left == keyevent->key()) {
            // 模拟空格键按下事件
            pressSpace(object);
            return true;
        }

        if ((Qt::AltModifier == keyevent->modifiers()) && Qt::Key_M == keyevent->key()) {
            /***add begin by ut001121 zhangmeng 20200825 模拟发送ContextMenu事件 修复BUG44282***/
            QPoint pos;
            QContextMenuEvent menuEvent(QContextMenuEvent::Keyboard, pos);
            qInfo() << "------------" << menuEvent.type();
            QApplication::sendEvent(object, &menuEvent);
            /***add end by ut001121***/

            return true;
        }

        return QApplication::notify(object, event);
    }

#if 0
    // 快捷键检测
    bool spont = event->spontaneous();
    //qInfo() <<event->type()<< spont<<classname;
    QString classname = object->metaObject()->className();
    if ((event->type() == QEvent::KeyPress || event->type() == QEvent::Shortcut)
            /*&& QString(object->metaObject()->className()) == "MainWindow"*/) {

        QKeyEvent *keyevent = static_cast<QKeyEvent *>(event);
        QString keyString;
        if (!keyevent->spontaneous() && n == "QWidgetWindow")
            // return  true;

            if (event->type() == QEvent::Shortcut) {
                QShortcutEvent *sevent = static_cast<QShortcutEvent *>(event);
                keyString = QKeySequence(sevent->key()).toString(QKeySequence::PortableText);
                // qInfo()<<"Shortcut"<<sevent->key();
            }

        int uKey = keyevent->key();
        Qt::Key key = static_cast<Qt::Key>(uKey);
        if (key == Qt::Key_unknown)
            // nothing {unknown key}


            if (key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt)
                return false;

        //获取修饰键(Ctrl,Alt,Shift)的状态
        Qt::KeyboardModifiers modifiers = keyevent->modifiers();
        //判断某个修饰键是否被按下化
        if (modifiers & Qt::ShiftModifier)
            uKey += Qt::SHIFT;
        if (modifiers & Qt::ControlModifier)
            uKey += Qt::CTRL;
        if (modifiers & Qt::AltModifier)
            uKey += Qt::ALT;

        QString keyString2 = QKeySequence(uKey).toString(QKeySequence::PortableText);
        qInfo() << keyevent->type() << keyString2 << keyString << object << keyevent->spontaneous() << classname << keyevent->key()
                << keyevent->nativeScanCode() << keyevent->nativeVirtualKey() << keyevent->nativeModifiers();

    }
#endif


#if 0
    // 焦点检测
    if (event->type() == QEvent::FocusIn)
        qInfo() << "FocusIn:" << object;

#endif

    return QApplication::notify(object, event);
}

void TerminalApplication::pressSpace(QObject *obj)
{
    // 模拟空格键按下事件
    QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
    QApplication::sendEvent(obj, &pressSpace);
    // 设置定时
    QTimer::singleShot(80, this, [obj]() {
        // 模拟空格键松开事件
        QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(obj, &releaseSpace);
    });
}
