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
#include "windowsmanager.h"
#include "shortcutmanager.h"
#include "settings.h"


#include <DSettingsDialog>
#include <DDialog>
#include <QObject>
#include <QSharedMemory>

DWIDGET_USE_NAMESPACE
struct ShareMemoryInfo{
    int enableCreateTerminal = 0;
    int TerminalsCount = 0;
};

class Service : public QObject
{
    Q_OBJECT
public:
    static Service *instance();
    ~Service();
    void init();
    // 初始化设置框
    void initSetting();

    // 显示设置框
    void showSettingDialog(MainWindow *pOwner);
    // 显示设置快捷键冲突弹窗
    void showShortcutConflictMsgbox(QString txt);
    // 设置框是否显示
    bool isSettingDialogVisible()
    {
        return m_settingDialog ? m_settingDialog->isVisible() : false;
    }
    //获取设置框的所有者
    MainWindow *getSettingOwner()
    {
        return m_settingOwner;
    }
    //重置设置框的所有者
    void resetSettingOwner()
    {
        m_settingOwner = nullptr;
    }

    // 是否允许继续创建
    bool isCountEnable();

    bool getIsDialogShow() const;
    void setIsDialogShow(QWidget *parent, bool isDialogShow);

    bool getEnable() ;
    void updateShareMemoryCount(int count);
    int  getShareMemoryCount();
    bool setMemoryEnable(bool enable);
    bool getMemoryEnable();
    void releaseShareMemory();

signals:
    void refreshCommandPanel(QString oldCmdName, QString newCmdName);
    // 切换编码列表的编码
    void checkEncode(QString encode);
    // 当前终端切换
    void currentTermChange(QWidget *term);

public slots:
    void Entry(QStringList arguments);
private:
    explicit Service(QObject *parent = nullptr);
    static Service *pService ;
    // 设置框 全局唯一显示
    DSettingsDialog *m_settingDialog = nullptr;
    //设置框的所有者
    MainWindow *m_settingOwner = nullptr;
    // 设置框，快捷键冲突弹窗
    DDialog *m_settingShortcutConflictDialog = nullptr;
    // 雷神用来判断是否有弹窗显示
    bool m_isDialogShow = false;
    // 是否允许创建新的窗口
    bool m_enable = false;
    QSharedMemory *m_enableShareMemory = nullptr;
    // 这个指针实际上与上面指针指向同一地址，不需要二次释放
    ShareMemoryInfo *m_pShareMemoryInfo = nullptr;
};

#endif // SERVICE_H
