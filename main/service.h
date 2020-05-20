#ifndef SERVICE_H
#define SERVICE_H
/*******************************************************************************
 1. @类名:    Service
 2. @作者:    ut000439 王培利
 3. @日期:    2020-05-19
 4. @说明:    全局的后台服务类．
　　　　　　　　管理所有底层数据的加载
　　　　　　　　管理所有特殊的通知信号
　　　　　　　　管理所有程序入口
*******************************************************************************/
#include "termproperties.h"
#include <DSettingsDialog>
#include <QObject>

DWIDGET_USE_NAMESPACE

class Service : public QObject
{
    Q_OBJECT
public:
    static Service *instance();
    void init();

    // 显示设置框
    void showSettingDialog();

signals:

public slots:
    void Entry(QStringList arguments);
private:
    explicit Service(QObject *parent = nullptr);
    static Service *pService ;
    // 设置框 全局唯一显示
    DSettingsDialog *m_settingDialog = nullptr;
};

#endif // SERVICE_H
