/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:  wangpeili<wangpeili@uniontech.com>
 *
 * Maintainer:wangpeili<wangpeili@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "service.h"
#include "utils.h"
#include "define.h"

#include <DSettings>
#include <DSettingsGroup>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DSysInfo>

#include <QDebug>
#include <QDateTime>
#include <QCheckBox>
#include <QLabel>

Service *Service::pService = new Service();
Service *Service::instance()
{
    return pService;
}

Service::~Service()
{
    qDebug() << __FUNCTION__;
    if (nullptr != m_atspiThread) {
        // 结束线程
        m_atspiThread->stopThread();
        delete m_atspiThread;
    }
    releaseShareMemory();
    // 判断非平板模式才会delete(因为平板模式设置了对话框的parent窗口)
    if (!m_isTabletMode && nullptr != m_settingDialog) {
        delete m_settingDialog;
    }
    if (nullptr != m_settingOwner) {
        delete m_settingOwner;
    }
    if (nullptr != m_settingShortcutConflictDialog) {
        delete m_settingShortcutConflictDialog;
    }
    if (nullptr != m_customThemeSettingDialog) {
        delete m_customThemeSettingDialog;
        m_customThemeSettingDialog = nullptr;
    }

    qDebug() << "service release finish!";
}

/*******************************************************************************
 1. @函数:    init
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    初始化
*******************************************************************************/
void Service::init()
{
    /******** Modify by ut000610 daizhengwen 2020-07-09:在linux上兼容快捷键 Begin***************/
    // 初始化qt-at-spi
    // 该线程负责在linux上兼容快捷键，如ctrl+shift+? （服务器版暂不支持此兼容）
    m_atspiThread = new AtspiDesktop;
    // 运行线程
    m_atspiThread->start();
    /********************* Modify by ut000610 daizhengwen End ************************/
    // 初始化配置
    Settings::instance()->init();
    // 初始化自定义快捷键
    ShortcutManager::instance()->initShortcuts();
    // 初始化远程管理数据
    ServerConfigManager::instance()->initServerConfig();

    // 主进程：共享内存如果不存在即创建
    if (!m_enableShareMemory->attach()) {
        m_enableShareMemory->create(sizeof(ShareMemoryInfo));
        qDebug() << "m_enableShareMemory create" << m_enableShareMemory->key();
    }
    // 创建好以后，保持共享内存连接，防止释放。
    m_enableShareMemory->attach();
    // 主进程：首次赋值m_pShareMemoryInfo
    m_pShareMemoryInfo = static_cast<ShareMemoryInfo *>(m_enableShareMemory->data());
    // 主进程：首次连接设置默认值为false
    setMemoryEnable(false);
    // 清理共享内存
    setSubAppStartTime(0);
    qDebug() << "All init data complete! m_enableShareMemory is" << m_enableShareMemory->key();

    //监听窗口特效变化
    listenWindowEffectSwitcher();
}

/*******************************************************************************
 1. @函数:    initSetting
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-06-05
 4. @说明:    初始化设置框，在窗口现实后初始化，使第一次出现设置框不至于卡顿
*******************************************************************************/
void Service::initSetting(MainWindow *pOwner)
{
    if (nullptr != m_settingDialog) {
        return;
    }
    QDateTime startTime = QDateTime::currentDateTime();
    m_settingDialog = new TabletSettingsDialog(pOwner);
    m_settingDialog->setObjectName("SettingDialog");//Add by ut001000 renfeixiang 2020-08-13
    // 关闭后将指针置空，下次重新new
    connect(m_settingDialog, &TabletSettingsDialog::finished, this, [ = ](int result) {
        Q_UNUSED(result)
        //激活设置框的有拥者
        if (m_settingOwner) {
            m_settingOwner->activateWindow();
            m_settingOwner->focusCurrentPage();
        }
    });
    // 关闭时delete
    m_settingDialog->widgetFactory()->registerWidget("fontcombobox", Settings::createFontComBoBoxHandle);
    m_settingDialog->widgetFactory()->registerWidget("slider", Settings::createCustomSliderHandle);
    m_settingDialog->widgetFactory()->registerWidget("spinbutton", Settings::createSpinButtonHandle);
    m_settingDialog->widgetFactory()->registerWidget("shortcut", Settings::createShortcutEditOptionHandle);
    m_settingDialog->widgetFactory()->registerWidget("tabformatedit", Settings::createTabTitleFormatOptionHandle);
    m_settingDialog->widgetFactory()->registerWidget("remotetabformatedit", Settings::createRemoteTabTitleFormatOptionHandle);
    m_settingDialog->widgetFactory()->registerWidget("shellconfigcombox", Settings::createShellConfigComboxOptionHandle);
    // 将数据重新读入
    m_settingDialog->updateSettings(Settings::instance()->settings);
    // 设置窗口模态为没有模态，不阻塞窗口和进程
    m_settingDialog->setWindowModality(Qt::NonModal);
    // 非平板模式设置对话框窗口标志
    if (!m_isTabletMode) {
        // 让设置与窗口等效，隐藏后显示就不会被遮挡
        m_settingDialog->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
    }
    moveToCenter(m_settingDialog);
    QDateTime endTime = QDateTime::currentDateTime();
    qDebug() << "Setting init cost time " << endTime.toMSecsSinceEpoch() - startTime.toMSecsSinceEpoch() << "ms";

    //判断为UOS服务器版本时，隐藏透明度/背景模糊选项
    if (DSysInfo::deepinType() == DSysInfo::DeepinServer) {
        showHideOpacityAndBlurOptions(false);
        return;
    }

    showHideOpacityAndBlurOptions(isWindowEffectEnabled());
}

/*******************************************************************************
 1. @函数:    showHideOpacityAndBlurOptions
 2. @作者:    ut000438 王亮
 3. @日期:    2020-06-24
 4. @说明:   显示/隐藏设置的透明度和背景模糊选项-- 仅UOS服务器版本使用
*******************************************************************************/
void Service::showHideOpacityAndBlurOptions(bool isShow)
{
    QWidget *rightFrame = m_settingDialog->findChild<QWidget *>("RightFrame");
    if (nullptr == rightFrame) {
        qDebug() << "can not found RightFrame in QWidget";
        return;
    }

    QList<QWidget *> rightWidgetList = rightFrame->findChildren<QWidget *>();

    for (int i = 0; i < rightWidgetList.size(); i++) {
        QWidget *widget = rightWidgetList.at(i);
        if (widget == nullptr) {
            continue;
        }
        if (strcmp(widget->metaObject()->className(), "QCheckBox") == 0) {
            QString checkText = (qobject_cast<QCheckBox *>(widget))->text();
            if (checkText == QObject::tr("Blur background")) {
                QWidget *optionWidget = widget;
                QWidget *parentWidget = widget->parentWidget();
                qDebug() << parentWidget << endl;
                if (parentWidget && strcmp(parentWidget->metaObject()->className(), "Dtk::Widget::DFrame") == 0) {
                    optionWidget = parentWidget;
                }
                if (isShow) {
                    optionWidget->show();
                } else {
                    optionWidget->hide();
                }
            }
        } else if (strcmp(widget->metaObject()->className(), "Dtk::Widget::DSlider") == 0) {
            QWidget *optionWidget = widget;
            QWidget *parentWidget = widget->parentWidget();
            qDebug() << parentWidget << endl;
            if (parentWidget && strcmp(parentWidget->metaObject()->className(), "Dtk::Widget::DFrame") == 0) {
                optionWidget = parentWidget;
            }
            if (isShow) {
                optionWidget->show();
            } else {
                optionWidget->hide();
            }
        } else if (strcmp(widget->metaObject()->className(), "QLabel") == 0) {
            QString lblText = (qobject_cast<QLabel *>(widget))->text();
            if (lblText == QObject::tr("Opacity")) {
                QWidget *optionWidget = widget;
                QWidget *parentWidget = widget->parentWidget();
                qDebug() << parentWidget << endl;
                if (parentWidget && strcmp(parentWidget->metaObject()->className(), "Dtk::Widget::DFrame") == 0) {
                    optionWidget = parentWidget;
                }
                if (isShow) {
                    optionWidget->show();
                } else {
                    optionWidget->hide();
                }
            }
        } else {
            //do nothing
        }
    }
}

/*******************************************************************************
 1. @函数:    listenWindowEffectSwitcher
 2. @作者:    ut000438 王亮
 3. @日期:    2020-06-24
 4. @说明:   监听窗口特效开关对应DBus信号，并实时显示/隐藏设置的透明度和背景模糊选项
*******************************************************************************/
void Service::listenWindowEffectSwitcher()
{
    if (nullptr == m_wmSwitcher) {
        m_wmSwitcher = new WMSwitcher(WMSwitcherService, WMSwitcherPath, QDBusConnection::sessionBus(), this);
        m_wmSwitcher->setObjectName("WMSwitcher");//Add by ut001000 renfeixiang 2020-08-13
        connect(m_wmSwitcher, &WMSwitcher::WMChanged, this, [this](const QString & wmName) {

            qDebug() << "changed wm name:" << wmName;
            bool isWinEffectEnabled = false;
            if (wmName == "deepin wm") {
                isWinEffectEnabled = true;
            }

            showHideOpacityAndBlurOptions(isWinEffectEnabled);

            emit Service::instance()->onWindowEffectEnabled(isWinEffectEnabled);
        }, Qt::QueuedConnection);
    }
}

/*******************************************************************************
 1. @函数:    isWindowEffectEnabled
 2. @作者:    ut000438 王亮
 3. @日期:    2020-06-24
 4. @说明:   判断当前是否开启窗口特效  开启-true 关闭-false
*******************************************************************************/
bool Service::isWindowEffectEnabled()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(WMSwitcherService, WMSwitcherPath, WMSwitcherService, "CurrentWM");

    QDBusMessage response = QDBusConnection::sessionBus().call(msg);
    if (response.type() == QDBusMessage::ReplyMessage) {
        QList<QVariant> list = response.arguments();
        QString wmName = list.first().toString();
        if (wmName == "deepin wm") {
            qDebug() << "The window effects is on";
            return true;
        }
    } else {
        qDebug() << "call CurrentWM Fail!" << response.errorMessage();
    }

    qDebug() << "The window effects is off";
    return false;
}

/*******************************************************************************
 1. @函数:    getEntryTime
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    获取主程序初始进入的时间
*******************************************************************************/
qint64 Service::getEntryTime()
{
    return m_EntryTime;
}

/*******************************************************************************
 1. @函数:    getShells
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-11-26
 4. @说明:    从/etc/shells获取shell列表
*******************************************************************************/
QMap<QString, QString> Service::getShells()
{
    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    // 清空原有数据
    m_shellsMap.clear();
    // 需要读取/etc/shells
    QFile shellsInfo(QStringLiteral("/etc/shells"));
    if (shellsInfo.open(QIODevice::ReadOnly)) {
        // 只读
        QTextStream stream(&shellsInfo);
        QString shellLine;
        // 循环读取
        do {
            QString shellPath;
            shellLine = stream.readLine();
            // 忽略注释
            if (!shellLine.startsWith(QLatin1String("#")) && !shellLine.isNull() && !shellLine.isEmpty()) {
                // 获取shell所在目录
                shellPath = shellLine;
                // 获取shell进程名称
                QStringList shellPaths = shellPath.split(QLatin1String("/"));
                QString shellProgram = shellPaths.back();
                // 添加数据入map
                m_shellsMap.insert(shellProgram, shellPath);
                // qDebug() << "shell : " << shellProgram << " path : " << shellPath;
            }
        } while (!shellLine.isNull());
    } else {
        // 读取数据失败报错
        qDebug() << "read /etc/shells fail! error : " << shellsInfo.error();
    }
    // 关闭文件
    shellsInfo.close();
    qint64 endTime = QDateTime::currentMSecsSinceEpoch();
    qDebug() << "read /etc/shells cost time : " << endTime - startTime;
    return m_shellsMap;
}

/*******************************************************************************
 1. @函数:    shellsMap
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-11-26
 4. @说明:    获取shellsMap
*******************************************************************************/
QMap<QString, QString> Service::shellsMap()
{
    return m_shellsMap;
}

/*******************************************************************************
 1. @函数:    showSettingDialog
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-05-20
 4. @说明:    唯一显示设置框
*******************************************************************************/
void Service::showSettingDialog(MainWindow *pOwner)
{
    QDateTime startTime = QDateTime::currentDateTime();
    // 第一次初始化dialog
    initSetting(pOwner);
    //保存设置框的有拥者
    m_settingOwner = pOwner;
    if (nullptr != m_settingDialog) {
        //雷神需要让窗口置顶，可是普通窗口不要
        if (m_settingOwner == WindowsManager::instance()->getQuakeWindow()) {
            m_settingDialog->setWindowFlag(Qt::WindowStaysOnTopHint);
        } else {
            // 雷神窗口失去焦点自动隐藏
            if (WindowsManager::instance()->getQuakeWindow()) {
                WindowsManager::instance()->getQuakeWindow()->onAppFocusChangeForQuake();
            }
            m_settingDialog->setWindowFlag(Qt::WindowStaysOnTopHint, false);
        }
        // 显示窗口
//        m_settingDialog->move(m_settingDialog->pos());
        /******** Add by ut001000 renfeixiang 2020-06-15:增加 每次显示设置窗口时，执行等宽字体出来 Begin***************/
        Settings::instance()->HandleWidthFont();
        FontFilter::instance()->HandleWidthFont();
        /******** Add by ut001000 renfeixiang 2020-06-15:增加 每次显示设置窗口时，执行等宽字体出来 End***************/
        // 重新加载shell配置数据
        Settings::instance()->reloadShellOptions();
        m_settingDialog->show();
    } else {
        qDebug() << "No setting dialog.";
        return;
    }

    // 若设置窗口已显示，则激活窗口
    if (!m_settingDialog->isActiveWindow()) {
        m_settingDialog->activateWindow();
    }
    QDateTime endTime = QDateTime::currentDateTime();
    qDebug() << "Setting show cost time " << endTime.toMSecsSinceEpoch() - startTime.toMSecsSinceEpoch() << "ms";
    QString strSettingsShowTime = GRAB_POINT + LOGO_TYPE + SHOW_SETTINGS_TIME + QString::number(endTime.toMSecsSinceEpoch() - startTime.toMSecsSinceEpoch());
    qInfo() << qPrintable(strSettingsShowTime);
}

/*******************************************************************************
 1. @函数:    showCustomThemeSettingDialog
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-12-01
 4. @说明:    显示自定义主题设置框
*******************************************************************************/
void Service::showCustomThemeSettingDialog(MainWindow *pOwner)
{
    //保存设置框的有拥者
    m_settingOwner = pOwner;
    if (nullptr != m_customThemeSettingDialog) {
        //雷神需要让窗口置顶，可是普通窗口不要
        if (m_settingOwner == WindowsManager::instance()->getQuakeWindow()) {
            m_customThemeSettingDialog->setWindowFlag(Qt::WindowStaysOnTopHint);
        } else {
            // 雷神窗口失去焦点自动隐藏
            if (WindowsManager::instance()->getQuakeWindow()) {
                WindowsManager::instance()->getQuakeWindow()->onAppFocusChangeForQuake();
            }
            m_customThemeSettingDialog->setWindowFlag(Qt::WindowStaysOnTopHint, false);
        }
    } else {
        m_customThemeSettingDialog = new CustomThemeSettingDialog();

        connect(m_customThemeSettingDialog, &CustomThemeSettingDialog::finished, this, [ & ](int result) {
            if (result == CustomThemeSettingDialog::Accepted) {
                qDebug() << "CustomThemeSettingDialog::Accepted";
                m_settingOwner->switchThemeAction(m_settingOwner->m_themeCustomAction, Settings::instance()->m_configCustomThemePath);
                return;
            } else {
                qDebug() << "CustomThemeSettingDialog::Rejected";
            }

        });
        // 设置窗口模态为没有模态，不阻塞窗口和进程
        m_customThemeSettingDialog->setWindowModality(Qt::NonModal);
        // 让设置与窗口等效，隐藏后显示就不会被遮挡
        m_customThemeSettingDialog->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
        moveToCenter(m_customThemeSettingDialog);
    }

    m_customThemeSettingDialog->show();

    // 若设置窗口已显示，则激活窗口
    if (!m_customThemeSettingDialog->isActiveWindow()) {
        m_customThemeSettingDialog->activateWindow();
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
    /******** Modify by ut000610 daizhengwen 2020-05-27: 出现提示和快捷键显示不一致的问题 bug#28507****************/
    // fix#bug 37399
    for (QString key : ShortcutManager::instance()->m_mapReplaceText.keys()) {
        if (txt.contains(key)) {
            txt.replace(key, ShortcutManager::instance()->m_mapReplaceText[key]);
        }
    }
    /********************* Modify by ut000610 daizhengwen End ************************/
    // 若没有设置弹框则退出，谈不上显示设置的快捷键冲突
    if (nullptr == m_settingDialog) {
        return;
    }
    // 若没有弹窗，初始化
    if (nullptr == m_settingShortcutConflictDialog) {
        m_settingShortcutConflictDialog = new DDialog(m_settingDialog);
        m_settingShortcutConflictDialog->setObjectName("ServiceSettingShortcutConflictDialog");// Add by ut001000 renfeixiang 2020-08-13
        connect(m_settingShortcutConflictDialog, &DDialog::finished, m_settingShortcutConflictDialog, [ = ]() {
            delete m_settingShortcutConflictDialog;
            m_settingShortcutConflictDialog = nullptr;
        });
        m_settingShortcutConflictDialog->setIcon(QIcon::fromTheme("dialog-warning"));
        /***mod by ut001121 zhangmeng 20200521 将确认按钮设置为默认按钮 修复BUG26960***/
        m_settingShortcutConflictDialog->addButton(QString(tr("OK")), true, DDialog::ButtonNormal);
    }
    m_settingShortcutConflictDialog->setTitle(QString(txt + QObject::tr("please set another one.")));
    m_settingShortcutConflictDialog->show();
    // 将冲突窗口移到窗口中央
    moveToCenter(m_settingShortcutConflictDialog);
}

/*******************************************************************************
 1. @函数:    isCountEnable
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-06-15
 4. @说明:    从term数量的角度判断是否允许继续创建
*******************************************************************************/
bool Service::isCountEnable()
{
    return WindowsManager::instance()->widgetCount() < MAXWIDGETCOUNT;
}

/*******************************************************************************
 1. @函数:    Entry
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    Service进入接口
*******************************************************************************/
void Service::Entry(QStringList arguments)
{
    m_EntryTime = QDateTime::currentDateTime().toMSecsSinceEpoch();

    // TODO: 暂时dtkgui版本还是5.4.0，等后面升级5.4.3以上才可以用这个开关
    bool isTabletMode = IS_TABLET_MODE;
    // 参数解析
    TermProperties properties;
    if (isTabletMode) {
        m_isTabletMode = isTabletMode;
        Utils::parseCommandLineTablet(arguments, properties);
    }
    else {
        Utils::parseCommandLine(arguments, properties);
    }

    // 雷神处理入口
    if (properties[QuakeMode].toBool()) {
        WindowsManager::instance()->runQuakeWindow(properties);
        return;
    }

    // 普通窗口处理入口
    WindowsManager::instance()->createNormalWindow(properties);
    return;
}

/*******************************************************************************
 1. @函数:    desktopWorkspaceSwitched
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-08-24
 4. @说明:    对桌面切换事件的处理
*******************************************************************************/
void Service::onDesktopWorkspaceSwitched(int curDesktop, int nextDesktop)
{
    qDebug() << __FUNCTION__ << curDesktop << nextDesktop;
    // 获取雷神窗口
    QuakeWindow *window = static_cast<QuakeWindow *>(WindowsManager::instance()->getQuakeWindow());
    // 没有雷神,直接返回
    if (nullptr == window) {
        return;
    }
    // 雷神在所在桌面是否显示
    bool isQuakeVisible = window->isShowOnCurrentDesktop();
    // 判断下一个窗口是否是雷神所在的窗口
    if (nextDesktop != window->getDesktopIndex()) {
        // 下一个桌面不是当前桌面,隐藏
        if (isQuakeVisible) {
            // 根据雷神显隐,判断此时雷神的显隐,若已经隐了,不用再隐
            window->hide();
        }
        window->hide();
    } else {
        // 下一个是雷神的窗口
        if (isQuakeVisible) {
            // 根据雷神显隐,判断此时雷神的显隐
            window->show();
            window->activateWindow();
        }
        // 另一种情况,雷神已经是隐藏状态不用再影藏
    }
}

Service::Service(QObject *parent) : QObject(parent)
{
    Utils::set_Object_Name(this);
    // 不同用户不能交叉使用共享内存，以及dbus, 所以共享内存的名字和登陆使用的用户有关。
    // 如sudo 用户名为root, 使用的配置也是root的配置。
    QString ShareMemoryName = QString(getenv("LOGNAME")) + "_enableCreateTerminal";
    //qDebug() << "ShareMemoryName: " << ShareMemoryName;
    m_enableShareMemory = new QSharedMemory(ShareMemoryName);
    m_enableShareMemory->setObjectName("EnableShareMemory");// Add by ut001000 renfeixiang 2020-08-13
}

/*******************************************************************************
 1. @函数:    getEnable
 2. @作者:    ut000439 王培利
 3. @日期:    2020-06-17
 4. @说明:    子进程获取是否可以创建窗口许可，获取到权限立即将标志位置为false
              增加子进程启动的时间，如果可以创建，把该时间写入共享内存，当创建mainwindow的时候，取走这个数据
*******************************************************************************/
bool Service::getEnable(qint64 time)
{
    if (!isCountEnable()) {
        return false;
    }
    // 如果共享内存无法访问？这是极为异常的情况。正常共享内存的建立由主进程创建，并保持attach不释放。
    if (!m_enableShareMemory->attach()) {
        qDebug() << "[sub app] m_enableShareMemory  can't attach" << m_enableShareMemory->key();
        return  false;
    }
    // sub app首次赋值m_pShareMemoryInfo
    m_pShareMemoryInfo = static_cast<ShareMemoryInfo *>(m_enableShareMemory->data());
    if (getShareMemoryCount() >= MAXWIDGETCOUNT) {
        qDebug() << "[sub app] current Terminals count = " << m_pShareMemoryInfo->TerminalsCount
                 << ", can't create terminal any more.";
        return false;
    }
    qDebug() << "[sub app] current Terminals count = " << m_pShareMemoryInfo->TerminalsCount;
    // 如果标志位为false，则表示正在创建窗口，不可以再创建
    if (!getMemoryEnable()) {
        releaseShareMemory();
        qDebug() << "[sub app] server m_enableShareMemory  is busy create!";
        return false;
    }
    // 可以创建了，立马将标识位置为false.
    setMemoryEnable(false);
    setSubAppStartTime(time);
    releaseShareMemory();
    return true;
}
/*******************************************************************************
 1. @函数:    setSubAppStartTime
 2. @作者:    ut000439 王培利
 3. @日期:    2020-08-08
 4. @说明:    设置子进程启动时间到共享内存
*******************************************************************************/
void Service::setSubAppStartTime(qint64 time)
{
    // 如果共享内存无法访问？这是极为异常的情况。正常共享内存的建立由主进程创建，并保持attach不释放。
    if (!m_enableShareMemory->isAttached()) {
        qDebug() << "m_enableShareMemory  isAttached failed?????";
        return ;
    }

    m_pShareMemoryInfo->appStartTime = time;
    qDebug() << "[sub app] app Start Time = " << m_pShareMemoryInfo->appStartTime;

    return ;
}
/*******************************************************************************
 1. @函数:    getSubAppStartTime
 2. @作者:    ut000439 王培利
 3. @日期:    2020-08-08
 4. @说明:    获取子进程在共享的启动时间
*******************************************************************************/
qint64 Service::getSubAppStartTime()
{
    return  m_pShareMemoryInfo->appStartTime;
}
/*******************************************************************************
 1. @函数:    updateShareMemoryCount
 2. @作者:    ut000439 王培利
 3. @日期:    2020-07-02
 4. @说明:    ShareMemoryCount这个为当前总的终端数，但是雷神创建的第一个不包括在内
*******************************************************************************/
void Service::updateShareMemoryCount(int count)
{
    if (!m_enableShareMemory->isAttached()) {
        qDebug() << "m_enableShareMemory  isAttached failed?????" << m_enableShareMemory->key();
        return ;
    }

    m_pShareMemoryInfo->TerminalsCount = count;
    qDebug() << "[main app] TerminalsCount  set " << count;

    return  ;
}

/*******************************************************************************
 1. @函数:    getShareMemoryCount
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    获取共享内存计数
*******************************************************************************/
int Service::getShareMemoryCount()
{
    return m_pShareMemoryInfo->TerminalsCount;
}
/*******************************************************************************
 1. @函数:    setMemoryEnable
 2. @作者:    ut000439 王培利
 3. @日期:    2020-06-17
 4. @说明:    设置共享内存信息，1=true(主进程), 0=false(主进程首次或子进程获得许可以后)
*******************************************************************************/
bool Service::setMemoryEnable(bool enable)
{
    if (!m_enableShareMemory->isAttached()) {
        qDebug() << "m_enableShareMemory  isAttached failed?????" << m_enableShareMemory->key();
        return false;
    }
    if (enable) {
        m_pShareMemoryInfo->enableCreateTerminal = 1;
    } else {
        m_pShareMemoryInfo->enableCreateTerminal = 0;
    }
    qDebug() << "m_enableShareMemory set" << enable << m_pShareMemoryInfo->enableCreateTerminal;
    return  true;
}
/*******************************************************************************
 1. @函数:    releaseShareMemory
 2. @作者:    ut000439 王培利
 3. @日期:    2020-06-17
 4. @说明:    释放共享内存连接
*******************************************************************************/
void Service::releaseShareMemory()
{
    qDebug() << "[sub app] m_enableShareMemory released" << m_enableShareMemory->key();
    m_enableShareMemory->detach();
    m_enableShareMemory->deleteLater();
}
/*******************************************************************************
 1. @函数:    getMemoryEnable
 2. @作者:    ut000439 王培利
 3. @日期:    2020-06-17
 4. @说明:    获取共享内存标志， 1=true,0=false
*******************************************************************************/
bool Service::getMemoryEnable()
{
    if (m_pShareMemoryInfo->enableCreateTerminal == 0) {
        qDebug() << "[sub app] current m_enableShareMemory is false" << m_pShareMemoryInfo->enableCreateTerminal;
        return false;
    }
    qDebug() << "[sub app] current m_enableShareMemory is true" << m_pShareMemoryInfo->enableCreateTerminal;
    return  true;
}

/*******************************************************************************
 1. @函数:    getIsDialogShow
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    获取是否是对话框显示
*******************************************************************************/
bool Service::getIsDialogShow() const
{
    return m_isDialogShow;
}

/*******************************************************************************
 1. @函数:    setIsDialogShow
 2. @作者:    ut000439 wangpeili
 3. @日期:    2020-08-11
 4. @说明:    设置是否对话框显示
*******************************************************************************/
void Service::setIsDialogShow(QWidget *parent, bool isDialogShow)
{
    MainWindow *window = static_cast<MainWindow *>(parent);
    if (window == WindowsManager::instance()->getQuakeWindow()) {
        qDebug() << "QuakeWindow show or hide dialog " << isDialogShow;
        m_isDialogShow = isDialogShow;
    }

    if (true == isDialogShow) {
        // 对话框显示,终端窗口禁用
        window->setEnabled(false);
    } else {
        // 对话框隐藏或关闭,终端窗口启用,焦点重回终端
        window->setEnabled(true);
        window->focusCurrentPage();
    }
}

void Service::slotShowSettingsDialog()
{
    qInfo() << sender()->parent();
    MainWindow *mainWindow = qobject_cast<MainWindow *>(sender()->parent());
    Service::instance()->showSettingDialog(mainWindow);
}

/*******************************************************************************
 1. @函数:    获取虚拟键盘布局(geometry)
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-13
 4. @说明:    getVirtualKeyboardGeometry
*******************************************************************************/
QRect Service::getVirtualKeyboardGeometry()
{
    QRect rectZero(0,0,0,0);
    // 虚拟键盘不可见时，返回0
    if (!m_isVirtualKeyboardShow) {
        return rectZero;
    }

    QDBusInterface vkinterface(DUE_IM_DBUS_NAME, DUE_IM_DBUS_PATH, DUE_IM_DBUS_INTERFACE, QDBusConnection::sessionBus());

    QVariant varGeometry = vkinterface.property("geometry");
    if (varGeometry.isValid()) {
        qDebug() << "geometry is: " << varGeometry;
        m_virtualKeyBoardRect = varGeometry.toRect();
        return m_virtualKeyBoardRect;
    }

    return rectZero;
}

/*******************************************************************************
 1. @函数:    getVirtualKeyboardHeight
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-13
 4. @说明:    获取虚拟键盘高度
*******************************************************************************/
int Service::getVirtualKeyboardHeight()
{
    // 虚拟键盘不可见时，返回0
    if (!m_isVirtualKeyboardShow) {
        return 0;
    }

    // 获取虚拟键盘高度
    if (-1 == m_virtualKeyboardHeight) {
        m_virtualKeyboardHeight = getVirtualKeyboardGeometry().height();
    }

    return m_virtualKeyboardHeight;
}

/*******************************************************************************
 1. @函数:    isVirtualKeyboardShow
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-13
 4. @说明:    判断虚拟键盘是否显示
*******************************************************************************/
bool Service::isVirtualKeyboardShow()
{
    // 非平板模式，不存在虚拟键盘
    bool isTabletMode = IS_TABLET_MODE;
    if (!isTabletMode) {
        return false;
    }

    return m_isVirtualKeyboardShow;
}

/*******************************************************************************
 1. @函数:    setVirtualKeyboardShow
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-13
 4. @说明:    设置虚拟键盘是否显示
*******************************************************************************/
void Service::setVirtualKeyboardShow(bool isShow)
{
    m_isVirtualKeyboardShow = isShow;
}

/*******************************************************************************
 1. @函数:    setTitleBarHeight
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-13
 4. @说明:    设置的标题栏高度来源于DMainWindow
*******************************************************************************/
void Service::setTitleBarHeight(int titleBarHeight)
{
    m_titleBarHeight = titleBarHeight;
}

/*******************************************************************************
 1. @函数:    getTitleBarHeight
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-13
 4. @说明:    获取DMainWindow标题栏高度
*******************************************************************************/
int Service::getTitleBarHeight()
{
    return m_titleBarHeight;
}

/*******************************************************************************
 1. @函数:    setIsPanelMovingBack
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-27
 4. @说明:    设置插件面板是否正在进行返回动画
*******************************************************************************/
void Service::setIsPanelMovingBack(bool isPanelMovingBack)
{
    m_isPanelMovingBack = isPanelMovingBack;
}

/*******************************************************************************
 1. @函数:    isPanelMovingBack
 2. @作者:    ut000438 王亮
 3. @日期:    2021-01-27
 4. @说明:    获取插件面板是否正在进行返回动画
*******************************************************************************/
bool Service::isPanelMovingBack()
{
    return m_isPanelMovingBack;
}
