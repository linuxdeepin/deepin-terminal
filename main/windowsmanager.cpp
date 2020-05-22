#include "windowsmanager.h"
#include "dbusmanager.h"
#include "utils.h"

#include <QDebug>

WindowsManager *WindowsManager::pManager = new WindowsManager();
WindowsManager *WindowsManager::instance()
{
    return  pManager;
}

void WindowsManager::runQuakeWindow(TermProperties properties)
{
    if (m_quakeWindow == nullptr) {
        qDebug() << "runQuakeWindow :create";
        m_quakeWindow = new MainWindow(properties);
        m_quakeWindow->show();
        return;
    }
    // Alt+F2的显隐功能实现点
    quakeWindowShowOrHide();
}

void WindowsManager::quakeWindowShowOrHide()
{
    //MainWindow *mainWindow = getMainWindow();
    qDebug() << "ShowOrHide" << m_quakeWindow->winId();

    // 没有显示，就显示．
    if (!m_quakeWindow->isVisible()) {
        qDebug() << "!mainWindow  isVisible now show !" << m_quakeWindow->winId();
        m_quakeWindow->show();
    }

    // 没有激活就激活
    if (!m_quakeWindow->isActiveWindow()) {
        qDebug() << "QuakeWindow is activate, now activateWindow" << m_quakeWindow->winId();
        int index = DBusManager::callKDECurrentDesktop();
        if (index != -1 && m_quakeWindow->getDesktopIndex() != index) {
            // 不在同一个桌面
            DBusManager::callKDESetCurrentDesktop(m_quakeWindow->getDesktopIndex());
        }
        m_quakeWindow->activateWindow();
        return;
    }

    // 如果已经激活，那么就隐藏
    qDebug() << "isWinVisible mainWindow->isActiveWindow() : start hide" << m_quakeWindow->winId();
    m_quakeWindow->hide();
}

void WindowsManager::createNormalWindow(TermProperties properties)
{
    TermProperties newProperties = properties;
    if (m_normalWindowList.count() == 0) {
        newProperties[SingleFlag] = true;
    }
    MainWindow *newWindow = new MainWindow(newProperties);
    m_normalWindowList << newWindow;
    qDebug() << "createNormalWindow, cureent count = " << m_normalWindowList.count();
    newWindow->show();
}

void WindowsManager::onMainwindowClosed(MainWindow * window)
{
    if(window->isQuakeMode())
    {
        Q_ASSERT(window == m_quakeWindow);
        m_quakeWindow->deleteLater();
        m_quakeWindow = nullptr;
        return;
    }
    if(m_normalWindowList.contains(window))
    {
        m_normalWindowList.removeOne(window);
        window->deleteLater();
        return;
    }
    qDebug()<<"unkown windows closed?? "<<window;
}

WindowsManager::WindowsManager(QObject *parent) : QObject(parent)
{

}

int WindowsManager::widgetCount() const
{
    return m_widgetCount;
}

void WindowsManager::windowCountIncrease()
{
    ++m_widgetCount;
    qDebug() << "++ Window Count : " << m_widgetCount;
}

void WindowsManager::windowCountReduce()
{
    --m_widgetCount;
    qDebug() << "-- Window Count : " << m_widgetCount;
}
