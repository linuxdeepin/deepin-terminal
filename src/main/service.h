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

#include <com_deepin_wmswitcher.h>

using WMSwitcher = com::deepin::WMSwitcher;
#define WMSwitcherService "com.deepin.WMSwitcher"
#define WMSwitcherPath "/com/deepin/WMSwitcher"


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

    //判断当前是否开启窗口特效  开启-true 关闭-false
    bool isWindowEffectEnabled();

signals:
    void refreshCommandPanel(QString oldCmdName, QString newCmdName);
    // 切换编码列表的编码
    void checkEncode(QString encode);
    // 当前终端切换
    void currentTermChange(QWidget *term);

    void onWindowEffectEnabled(bool isEnabled);

public slots:
    void Entry(QStringList arguments);
private:
    explicit Service(QObject *parent = nullptr);
    //隐藏设置透明度和背景模糊选项-- 仅UOS服务器版本使用
    void hideOpacityAndBlurOptions();
    //监听窗口特效开关对应DBus信号，并实时禁用/启用透明度和背景模糊选项
    void listenWindowEffectSwitcher();
    //根据是否开启窗口特效开关，禁用/启用透明度和背景模糊选项
    void changeSettingControlStatus(bool isWindowEffectEnabled);

    static Service *pService ;
    // 设置框 全局唯一显示
    DSettingsDialog *m_settingDialog = nullptr;

    WMSwitcher *m_wmSwitcher = nullptr;
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
