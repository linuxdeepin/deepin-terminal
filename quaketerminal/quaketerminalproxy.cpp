#include "quaketerminalproxy.h"
#include "mainwindow.h"

#include <DLog>

#include <QApplication>
#include <DSettingsDialog>
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
    MainWindow *quakeWindow = getMainWindow();
    qDebug() << "ShowOrHide" << quakeWindow->winId();

    // 没有显示，就显示．
    if (!quakeWindow->isVisible()) {
        qDebug() << "!mainWindow  isVisible now show !" << quakeWindow->winId();
        quakeWindow->show();
    }

    // 没有激活就激活
    if (!quakeWindow->isActiveWindow()) {
        qDebug() << "QuakeWindow is activate, now activateWindow" << quakeWindow->winId();
        quakeWindow->activateWindow();
        return;
    }

    // 如果已经激活，那么就隐藏
    qDebug() << "isWinVisible mainWindow->isActiveWindow() : start hide" << quakeWindow->winId();
    quakeWindow->hide();
}

void QuakeTerminalProxy::hidePlugin()
{
    MainWindow *quakeWindow = getMainWindow();
    quakeWindow->showPluginChanged(MainWindow::PLUGIN_TYPE_NONE);
}
