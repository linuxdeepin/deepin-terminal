/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  wangpeili<wangpeili@uniontech.com>
 *
 * Maintainer:wangpeili<wangpeili@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "terminalapplication.h"
#include "mainwindow.h"
#include "service.h"

// qt
#include <QDebug>

TerminalApplication::TerminalApplication(int &argc, char *argv[]) : DApplication(argc, argv)
{
    loadTranslator();
    setOrganizationName("deepin");
    setOrganizationDomain("deepin.org");
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
    // app结束时，释放Service
    if (nullptr != Service::instance()) {
        delete Service::instance();
    }
}
/*******************************************************************************
 1. @函数:    setStartTime
 2. @作者:    ut000439 王培利
 3. @日期:    2020-08-08
 4. @说明:    把main初始的时间，设置为应用启动时间
*******************************************************************************/
void TerminalApplication::setStartTime(qint64 time)
{
    m_AppStartTime = time;
}
/*******************************************************************************
 1. @函数:    getStartTime
 2. @作者:    ut000439 王培利
 3. @日期:    2020-08-08
 4. @说明:    只有主进程在创建mainwindow的时候会用到。
*******************************************************************************/
qint64 TerminalApplication::getStartTime()
{
    return m_AppStartTime;
}
/*******************************************************************************
 1. @函数:    handleQuitAction
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    重写了app.quit为当前窗口close.
*******************************************************************************/
void TerminalApplication::handleQuitAction()
{
    qDebug() << "handleQuitAction";
    activeWindow()->close();
}

/*******************************************************************************
 1. @函数:    notify
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    // 该部分代码，大都 用于调试，待整理
*******************************************************************************/
bool TerminalApplication::notify(QObject *object, QEvent *event)
{
    // ALT+M = 右键
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyevent = static_cast<QKeyEvent *>(event);
        /***add begin by ut001121 zhangmeng 20200801 截获DPushButton控件回车按键事件并模拟空格键点击事件,用以解决回车键不响应的问题***/
        // 回车键
        // 恢复默认 添健按钮
        if ((object->metaObject()->className() == QStringLiteral("QPushButton")
                // 远程和自定义列表的返回按钮，编辑按钮
                || object->metaObject()->className() == QStringLiteral("IconButton")
                // 搜索框的上下搜索
                || object->metaObject()->className() == QStringLiteral("Dtk::Widget::DIconButton")
                // 设置里面的单选框
                || object->metaObject()->className() == QStringLiteral("QCheckBox")
                // 设置字体组合框
                || object->metaObject()->className() == QStringLiteral("QComboBox")
                // 设置窗口组合框
                || object->metaObject()->className() == QStringLiteral("ComboBox"))
                && (keyevent->key() == Qt::Key_Return || keyevent->key() == Qt::Key_Enter)) {
            DPushButton *pushButton = static_cast<DPushButton *>(object);
            // 模拟空格键按下事件
            pressSpace(pushButton);
            return true;
        }
        /***add end by ut001121***/
        // 左键
        // 远程和自定义列表的返回按钮 Key_Left
        if ((object->objectName() == QStringLiteral("CustomRebackButton")
                || object->objectName() == QStringLiteral("RemoteSearchRebackButton")
                || object->objectName() == QStringLiteral("RemoteGroupRebackButton"))
                && keyevent->key() == Qt::Key_Left) {
            DPushButton *pushButton = static_cast<DPushButton *>(object);
            // 模拟空格键按下事件
            pressSpace(pushButton);
            return true;
        }

        if ((keyevent->modifiers() == Qt::AltModifier) && keyevent->key() == Qt::Key_M) {
            // 光标中心点
            QPoint pos = QPoint(qApp->inputMethod()->cursorRectangle().x() + qApp->inputMethod()->cursorRectangle().width() / 2,
                                qApp->inputMethod()->cursorRectangle().y() + qApp->inputMethod()->cursorRectangle().height() / 2);

            qDebug() << "Alt+M has triggerd" << pos << qApp->inputMethod();
            // QPoint(0,0) 表示无法获取光标位置
            if (pos != QPoint(0, 0)) {
                QMouseEvent event1(QEvent::MouseButtonPress, pos, Qt::RightButton, Qt::NoButton, Qt::NoModifier);
                QCoreApplication::sendEvent(object, &event1);
            }

            return true;
        }

        return QApplication::notify(object, event);
    }
#if 0
    // 快捷键检测
    bool spont = event->spontaneous();
    //qDebug() <<event->type()<< spont<<classname;
    QString classname = object->metaObject()->className();
    if ((event->type() == QEvent::KeyPress || event->type() == QEvent::Shortcut)
            /*&& QString(object->metaObject()->className()) == "MainWindow"*/) {

        QKeyEvent *keyevent = static_cast<QKeyEvent *>(event);
        QString keyString;
        if (!keyevent->spontaneous() && n == "QWidgetWindow") {
            // return  true;
        }
        if (event->type() == QEvent::Shortcut) {
            QShortcutEvent *sevent = static_cast<QShortcutEvent *>(event);
            keyString = QKeySequence(sevent->key()).toString(QKeySequence::PortableText);
            // qDebug()<<"Shortcut"<<sevent->key();
        }

        int uKey = keyevent->key();
        Qt::Key key = static_cast<Qt::Key>(uKey);
        if (key == Qt::Key_unknown) {
            // nothing {unknown key}
        }

        if (key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt) {
            return false;
        }

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
        qDebug() << keyevent->type() << keyString2 << keyString << object << keyevent->spontaneous() << classname << keyevent->key()
                 << keyevent->nativeScanCode() << keyevent->nativeVirtualKey() << keyevent->nativeModifiers();

    }
#endif


#if 0
    // 焦点检测
    if (event->type() == QEvent::FocusIn) {
        qDebug() << "FocusIn:" << object;
    }
#endif

    return QApplication::notify(object, event);
}

/*******************************************************************************
 1. @函数:    pressSpace
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-06
 4. @说明:    模拟键盘space按压效果
*******************************************************************************/
void TerminalApplication::pressSpace(DPushButton *pushButton)
{
    // 模拟空格键按下事件
    QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
    QApplication::sendEvent(pushButton, &pressSpace);
    // 设置定时
    QTimer::singleShot(80, this, [pushButton]() {
        // 模拟空格键松开事件
        QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(pushButton, &releaseSpace);
    });
}
