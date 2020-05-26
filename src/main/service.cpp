#include "service.h"
#include "windowsmanager.h"
#include "utils.h"

#include <DSettings>
#include <DSettingsGroup>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>

#include <QDebug>

Service *Service::pService = new Service();
Service *Service::instance()
{
    return  pService;
}

void Service::init()
{
    ServerConfigManager::instance()->initServerConfig();
}
/*******************************************************************************
 1. @函数:    showSettingDialog
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-05-20
 4. @说明:    唯一显示设置框
*******************************************************************************/
void Service::showSettingDialog(MainWindow *pOwner)
{
    //保存设置框的有拥者
    m_settingOwner = pOwner;
    if (nullptr == m_settingDialog) {
        m_settingDialog = new DSettingsDialog();
        // 关闭后将指针置空，下次重新new
        connect(m_settingDialog, &DSettingsDialog::finished, this, [ = ](int result) {
            // 关闭时置空
            if (result == 0) {
                m_settingDialog = nullptr;
            }
            //激活设置框的有拥者
            if (m_settingOwner) {
                m_settingOwner->activateWindow();
                m_settingOwner->focusCurrentPage();
            }
        });
        // 关闭时delete
        m_settingDialog->setAttribute(Qt::WA_DeleteOnClose);
        m_settingDialog->widgetFactory()->registerWidget("fontcombobox", Settings::createFontComBoBoxHandle);
        m_settingDialog->widgetFactory()->registerWidget("slider", Settings::createCustomSliderHandle);
        m_settingDialog->widgetFactory()->registerWidget("spinbutton", Settings::createSpinButtonHandle);
        m_settingDialog->widgetFactory()->registerWidget("shortcut", Settings::createShortcutEditOptionHandle);
        // 将数据重新读入
        m_settingDialog->updateSettings(Settings::instance()->settings);
        // 设置窗口模态为没有模态，不阻塞窗口和进程
        m_settingDialog->setWindowModality(Qt::NonModal);
        // 让设置与窗口等效，隐藏后显示就不会被遮挡
        m_settingDialog->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
    }
    // 雷神需要让窗口置顶，可是普通窗口不要
    if (m_settingOwner == WindowsManager::instance()->getQuakeWindow()) {
        m_settingDialog->setWindowFlag(Qt::WindowStaysOnTopHint);
    } else {
        m_settingDialog->setWindowFlag(Qt::WindowStaysOnTopHint, false);
    }
    // 显示窗口
    m_settingDialog->show();
    // 若设置窗口已显示，则激活窗口
    if (!m_settingDialog->isActiveWindow()) {
        m_settingDialog->activateWindow();
    }
}

/*******************************************************************************
 1. @函数:    showShortcutConflictMsgbox
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-05-21
 4. @说明:    设置弹窗的快捷键冲突弹窗
*******************************************************************************/
void Service::showShortcutConflictMsgbox(QString txt)
{
    // 若没有设置弹框则退出，谈不上显示设置的快捷键冲突
    if (nullptr == m_settingDialog) {
        return;
    }
    // 若没有弹窗，初始化
    if (nullptr == m_settingShortcutConflictDialog) {
        m_settingShortcutConflictDialog = new DDialog(m_settingDialog);
        connect(m_settingShortcutConflictDialog, &DDialog::finished, m_settingShortcutConflictDialog, [ = ]() {
            m_settingShortcutConflictDialog = nullptr;
        });
        m_settingShortcutConflictDialog->setIcon(QIcon::fromTheme("dialog-warning"));
        /***mod by ut001121 zhangmeng 20200521 将确认按钮设置为默认按钮 修复BUG26960***/
        m_settingShortcutConflictDialog->addButton(QString(tr("OK")), true, DDialog::ButtonNormal);
    }
    m_settingShortcutConflictDialog->setTitle(QString(txt + QObject::tr("please set another one.")));
    m_settingShortcutConflictDialog->show();
}

void Service::Entry(QStringList arguments)
{
    // 到达窗口最大值，则返回，不做创建
    if (WindowsManager::instance()->widgetCount() == MAXWIDGETCOUNT) {
        return;
    }
    qDebug() << "dzw " << arguments;
    TermProperties properties = Utils::parseArgument( arguments);
    // 雷神处理入口
    if (properties[QuakeMode].toBool()) {
        WindowsManager::instance()->runQuakeWindow(properties);
        return;
    }
    // 普通窗口处理入口
    WindowsManager::instance()->createNormalWindow(properties);
    return;
}

Service::Service(QObject *parent) : QObject(parent)
{

}
