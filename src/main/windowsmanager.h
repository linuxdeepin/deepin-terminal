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

// 窗口最大数量
#define MAXWIDGETCOUNT 200

class WindowsManager : public QObject
{
    Q_OBJECT
public:
    static WindowsManager * instance();
    void runQuakeWindow(TermProperties properties);
    void quakeWindowShowOrHide();
    void createNormalWindow(TermProperties properties);

    TermWidgetPage *currentPage();
    void setCurrentPage(TermWidgetPage *page);

    // 窗口数量增加
    void windowCountIncrease();
    // 窗口数量减少
    void windowCountReduce();
    // 获取当前窗口数量
    int widgetCount() const;
    // 获取雷神窗口
    MainWindow *getQuakeWindow() {return m_quakeWindow;}

signals:

public slots:
    void onMainwindowClosed(MainWindow *);
private:
    QList<MainWindow *> m_normalWindowList;
    MainWindow * m_quakeWindow = nullptr;
    TermWidgetPage *m_currentPage = nullptr;
private:
    explicit WindowsManager(QObject *parent = nullptr);
    static WindowsManager *pManager;
    // 窗口数量
    int m_widgetCount = 0;
};

#endif // WINDOWSMANAGER_H
