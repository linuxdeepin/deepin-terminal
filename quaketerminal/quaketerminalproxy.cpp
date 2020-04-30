#include "quaketerminalproxy.h"
#include "mainwindow.h"

#include <DLog>

#include <QApplication>
#include <QDir>
#include <QtDBus>

QuakeTerminalProxy::QuakeTerminalProxy(QObject *parent) : QObject(parent)
{
    this->setObjectName("QuakeTerminalProxy");
}

QuakeTerminalProxy::~QuakeTerminalProxy()
{
}

MainWindow *getMainWindow()
{
    MainWindow *mainWin = nullptr;
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        mainWin = qobject_cast<MainWindow *>(w);
        if (mainWin) {
            return mainWin;
        }
    }
    return nullptr;
}

void QuakeTerminalProxy::ShowOrHide()
{
    MainWindow *mainWindow = getMainWindow();
    qDebug() << "ShowOrHide" << mainWindow->winId();

    // 没有显示，就显示．
    if (!mainWindow->isVisible()) {
        qDebug() << "!mainWindow  isVisible now show !" << mainWindow->winId();
        mainWindow->show();
    }

    // 没有激活就激活
    if (!mainWindow->isActiveWindow()) {
        qDebug() << "QuakeWindow is activate, now activateWindow" << mainWindow->winId();
        mainWindow->activateWindow();
        return;
    }

    // 如果已经激活，那么就隐藏
    qDebug() << "isWinVisible mainWindow->isActiveWindow() : start hide" << mainWindow->winId();
    mainWindow->hide();
}
