#include "service.h"
#include "utils.h"

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
    if (nullptr != m_settingDialog) {
        delete m_settingDialog;
    }
    if (nullptr != m_settingOwner) {
        delete m_settingOwner;
    }
    if (nullptr != m_settingShortcutConflictDialog) {
        delete m_settingShortcutConflictDialog;
    }
    qDebug() << "service release finish!";
}

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
        qDebug() << "m_enableShareMemory create";
    }
    // 创建好以后，保持共享内存连接，防止释放。
    m_enableShareMemory->attach();
    // 主进程：首次赋值m_pShareMemoryInfo
    m_pShareMemoryInfo = static_cast<ShareMemoryInfo *>(m_enableShareMemory->data());
    // 主进程：首次连接设置默认值为false
    setMemoryEnable(false);
    qDebug() << "All init data complete!";

    //监听窗口特效变化
    listenWindowEffectSwitcher();
}

/*******************************************************************************
 1. @函数:    initSetting
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-06-05
 4. @说明:    初始化设置框，在窗口现实后初始化，使第一次出现设置框不至于卡顿
*******************************************************************************/
void Service::initSetting()
{
    if (nullptr != m_settingDialog) {
        return;
    }
    QDateTime startTime = QDateTime::currentDateTime();
    m_settingDialog = new DSettingsDialog();
    // 关闭后将指针置空，下次重新new
    connect(m_settingDialog, &DSettingsDialog::finished, this, [ = ](int result) {
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
    // 将数据重新读入
    m_settingDialog->updateSettings(Settings::instance()->settings);
    // 设置窗口模态为没有模态，不阻塞窗口和进程
    m_settingDialog->setWindowModality(Qt::NonModal);
    // 让设置与窗口等效，隐藏后显示就不会被遮挡
    m_settingDialog->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
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
        return;
    }

    QList<QWidget *> rightWidgetList = rightFrame->findChildren<QWidget *>();

    for (int i = 0; i < rightWidgetList.size(); i++) {
        QWidget *widget = rightWidgetList.at(i);
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
            qDebug() << "窗口特效已开启";
            return true;
        }
    } else {
        qDebug() << "call CurrentWM Fail!" << response.errorMessage();
    }

    qDebug() << "窗口特效已关闭";
    return false;
}

/*******************************************************************************
 1. @函数:    showSettingDialog
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-05-20
 4. @说明:    唯一显示设置框
*******************************************************************************/
void Service::showSettingDialog(MainWindow *pOwner)
{
    // 第一次初始化dialog
    initSetting();
    QDateTime startTime = QDateTime::currentDateTime();
    //保存设置框的有拥者
    m_settingOwner = pOwner;
    if (nullptr != m_settingDialog) {
        /******** Modify by ut000610 daizhengwen 2020-06-23:每当点击设置，都重新new一个settingDialog Begin***************/
        delete m_settingDialog;
        m_settingDialog = nullptr;
        // 重新new dialog  ----由于设置文字大小，窗口不会自适应，所以需要重新new，如果dtk设置框随设置变化大小，此处可去
        initSetting();
        /********************* Modify by ut000610 daizhengwen End ************************/
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
        m_settingDialog->move(m_settingDialog->pos());
        /******** Add by ut001000 renfeixiang 2020-06-15:增加 每次显示设置窗口时，执行等宽字体出来 Begin***************/
        Settings::instance()->HandleWidthFont();
        FontFilter::instance()->HandleWidthFont();
        /******** Add by ut001000 renfeixiang 2020-06-15:增加 每次显示设置窗口时，执行等宽字体出来 End***************/
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

void Service::Entry(QStringList arguments)
{
    TermProperties properties;
    Utils::parseCommandLine(arguments, properties);

    // 雷神处理入口
    if (properties[QuakeMode].toBool()) {
        WindowsManager::instance()->runQuakeWindow(properties);
        return;
    }

//    // 到达窗口最大值，则返回，不做创建
//    if (WindowsManager::instance()->widgetCount() >= MAXWIDGETCOUNT) {
//        // 当前不创建，要将enable还原
//        Service::instance()->setEnable(true);
//        return;
//    }
    // 普通窗口处理入口
    WindowsManager::instance()->createNormalWindow(properties);
    return;
}

Service::Service(QObject *parent) : QObject(parent)
{
    // 不同用户不能交叉使用共享内存，以及dbus, 所以共享内存的名字和登陆使用的用户有关。
    // 如sudo 用户名为root, 使用的配置也是root的配置。
    QString ShareMemoryName = QString(getenv("LOGNAME")) + "_enableCreateTerminal";
    qDebug() << "ShareMemoryName: " << ShareMemoryName;
    m_enableShareMemory = new QSharedMemory(ShareMemoryName);
}

/*******************************************************************************
 1. @函数:    getEnable
 2. @作者:    ut000439 王培利
 3. @日期:    2020-06-17
 4. @说明:    子进程获取是否可以创建窗口许可，获取到权限立即将标志位置为false
*******************************************************************************/
bool Service::getEnable()
{
    if (!Service::instance()->isCountEnable()) {
        return false;
    }
    // 如果共享内存无法访问？这是极为异常的情况。正常共享内存的建立由主进程创建，并保持attach不释放。
    if (!m_enableShareMemory->attach()) {
        qDebug() << "[sub app] m_enableShareMemory  can't attach";
        return  false;
    }
    // sub app首次赋值m_pShareMemoryInfo
    m_pShareMemoryInfo = static_cast<ShareMemoryInfo *>(m_enableShareMemory->data());
    if (Service::instance()->getShareMemoryCount() >= MAXWIDGETCOUNT) {
        qDebug() << "[sub app] current Terminals count = " << m_pShareMemoryInfo->TerminalsCount
                 << ", can't create terminal any more.";
        return false;
    }
    qDebug() << "[sub app] current Terminals count = " << m_pShareMemoryInfo->TerminalsCount;
    // 如果标志位为false，则表示正在创建窗口，不可以再创建
    if (!Service::instance()->getMemoryEnable()) {
        Service::instance()->releaseShareMemory();
        qDebug() << "[sub app] server m_enableShareMemory  is busy create!";
        return false;
    }
    // 可以创建了，立马将标识位置为false.
    Service::instance()->setMemoryEnable(false);
    Service::instance()->releaseShareMemory();
    return true;
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
        qDebug() << "m_enableShareMemory  isAttached failed?????";
        return ;
    }

    m_pShareMemoryInfo->TerminalsCount = count;
    qDebug() << "[main app] TerminalsCount  set " << count;

    return  ;
}

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
        qDebug() << "m_enableShareMemory  isAttached failed?????";
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
    qDebug() << "[sub app] m_enableShareMemory released";
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

bool Service::getIsDialogShow() const
{
    return m_isDialogShow;
}

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
