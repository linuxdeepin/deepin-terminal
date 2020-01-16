#include "quaketerminalproxy.h"
#include "mainwindow.h"

#include <QApplication>
#include <QDir>
#include <QtDBus/QtDBus>

#include <DLog>

QuakeTerminalProxy::QuakeTerminalProxy(QObject *parent) : QObject(parent)
{
    qDebug() << "QuakeTerminalProxy init";
    this->setObjectName("QuakeTerminalProxy");
}

QuakeTerminalProxy::~QuakeTerminalProxy() {}

MainWindow *getMainWindow()
{
    MainWindow *mainWin = nullptr;
    foreach (QWidget *w, qApp->topLevelWidgets())
    {
        mainWin = qobject_cast< MainWindow * >(w);
        if (mainWin)
        {
            return mainWin;
        }
    }
    return nullptr;
}

void QuakeTerminalProxy::ShowOrHide()
{
    MainWindow *mainWindow = getMainWindow();
    qDebug() << "ShowOrHide" << mainWindow->winId();
    bool isWinVisible = mainWindow->isVisible();
    if (isWinVisible)
    {
        if (mainWindow->isActiveWindow())
        {
            qDebug() << "isWinVisible mainWindow->isActiveWindow() : start hide" << mainWindow->winId();
            mainWindow->hide();
        }
        else
        {
            if (mainWindow->isQuakeWindowActivated())
            {
                qDebug() << "isWinVisible mainWindow->isQuakeWindowActivated() : "
                         << mainWindow->isQuakeWindowActivated();
                mainWindow->setQuakeWindowActivated(false);
                mainWindow->hide();
                return;
            }

            qDebug() << "isWinVisible not mainWindow->isActiveWindow() : start activateWindow" << mainWindow->winId();
            mainWindow->activateWindow();
            mainWindow->setQuakeWindowActivated(true);
        }
    }
    else
    {
        qDebug() << "!isWinVisible show and activateWindow" << mainWindow->winId();
        mainWindow->show();
        mainWindow->raise();
        mainWindow->activateWindow();
        mainWindow->setQuakeWindowActivated(true);
    }
}
