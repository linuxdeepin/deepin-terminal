#ifndef WINDOWSMANAGER_H
#define WINDOWSMANAGER_H
/*******************************************************************************
 1. @类名:    WindowsManager
 2. @作者:    ut000439 王培利
 3. @日期:    2020-05-19
 4. @说明:    全局窗口管理，维护mainwindow列表．
　　　　　　　　普通窗口的创建，关闭．
　　　　　　　　雷神窗口的创建，关闭，显与隐
*******************************************************************************/

#include "termproperties.h"
#include "mainwindow.h"

#include <QObject>


class WindowsManager : public QObject
{
    Q_OBJECT
public:
    static WindowsManager * instance();
    void runQuakeWindow(TermProperties properties);
    void quakeWindowShowOrHide();
    void createNormalWindow(TermProperties properties);

signals:

public slots:
private:
    QList<MainWindow *> m_normalWindowList;
    MainWindow * m_quakeWindow = nullptr;
private:
    explicit WindowsManager(QObject *parent = nullptr);
    static WindowsManager * pManager;
};

#endif // WINDOWSMANAGER_H
