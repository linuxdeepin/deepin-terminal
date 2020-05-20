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

}

void Service::showSettingDialog()
{
    if (nullptr == m_settingDialog) {
        m_settingDialog = new DSettingsDialog();
        connect(m_settingDialog, &DSettingsDialog::finished, this, [ = ](int result) {
            if (result == 0) {
                m_settingDialog = nullptr;
            }
        });
        m_settingDialog->setAttribute(Qt::WA_DeleteOnClose);
        m_settingDialog->widgetFactory()->registerWidget("fontcombobox", Settings::createFontComBoBoxHandle);
        m_settingDialog->widgetFactory()->registerWidget("slider", Settings::createCustomSliderHandle);
        m_settingDialog->widgetFactory()->registerWidget("spinbutton", Settings::createSpinButtonHandle);
        m_settingDialog->widgetFactory()->registerWidget("shortcut", Settings::createShortcutEditOptionHandle);

        m_settingDialog->updateSettings(Settings::instance()->settings);
        m_settingDialog->setWindowModality(Qt::NonModal);
        m_settingDialog->show();
    } else {
        m_settingDialog->activateWindow();
    }

}

void Service::Entry(QStringList arguments)
{
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
