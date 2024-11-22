// SPDX-FileCopyrightText: 2019-2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "service.h"
#include "utils.h"
#include "define.h"
#include "eventlogutils.h"

#include <DSettings>
#include <DSettingsGroup>
#include <DSettingsOption>
#include <DSettingsWidgetFactory>
#include <DSysInfo>
#include <DWindowManagerHelper>
#include <DTitlebar>

#include <QDebug>
#include <QDateTime>
#include <QCheckBox>
#include <QLabel>
#include <QScroller>
#include <QJsonObject>
#include <QLoggingCategory>

Service *Service::g_pService = nullptr;

Q_DECLARE_LOGGING_CATEGORY(mainprocess)

Service *Service::instance()
{
    // qCDebug(mainprocess)<< "Enter Service::instance";
    if(nullptr == g_pService) {
        qCDebug(mainprocess) << "Creating new Service instance";
        g_pService = new Service();
    }
    return g_pService;
}

Service::~Service()
{
    qCDebug(mainprocess)<< "Enter Service destructor";
    if (nullptr != m_settingDialog) {
        qCDebug(mainprocess)<< "Branch: deleting setting dialog";
        delete m_settingDialog;
        m_settingDialog = nullptr;
    }
    if (nullptr != m_settingOwner) {
        qCDebug(mainprocess)<< "Branch: deleting setting owner";
        delete m_settingOwner;
        m_settingOwner = nullptr;
    }
    if (nullptr != m_settingShortcutConflictDialog) {
        qCDebug(mainprocess)<< "Branch: deleting shortcut conflict dialog";
        delete m_settingShortcutConflictDialog;
        m_settingShortcutConflictDialog = nullptr;
    }
    if (nullptr != m_customThemeSettingDialog) {
        qCDebug(mainprocess)<< "Branch: deleting custom theme setting dialog";
        delete m_customThemeSettingDialog;
        m_customThemeSettingDialog = nullptr;
    }
}

void Service::init()
{
    qCDebug(mainprocess) << "Initializing Service";
    // 初始化自定义快捷键
    ShortcutManager::instance()->initShortcuts();
    qCInfo(mainprocess) << "Shortcuts initialized";
    // 初始化远程管理数据
    ServerConfigManager::instance()->initServerConfig();
    qCInfo(mainprocess) << "Server config initialized";
    // 主进程：首次赋值m_pShareMemoryInfo
    listenWindowEffectSwitcher();
    listenDebuginfodOption();
}

void Service::releaseInstance()
{
    qCDebug(mainprocess)<< "Enter Service::releaseInstance";
    if (nullptr != g_pService) {
        qCDebug(mainprocess)<< "Branch: deleting service instance";
        delete g_pService;
        g_pService = nullptr;
    }
}

void Service::initSetting(MainWindow *pOwner)
{
    qCDebug(mainprocess)<< "Enter Service::initSetting";
    if (nullptr != m_settingDialog) {
        qCDebug(mainprocess)<< "Branch: setting dialog already exists";
        // 当前处于弹出状态，不调整焦点位置
        if (m_settingDialog->isVisible()) {
            qCDebug(mainprocess)<< "Branch: dialog is visible, returning";
            return;
        }

        //1050e版本：二次打开设置窗口，焦点在【关闭按钮】上（bug#104810）
        qCDebug(mainprocess)<< "Looking for title bar and scroll area";
        DTitlebar *titleBar = Utils::findWidgetByAccessibleName<DTitlebar *>(m_settingDialog, "DSettingTitleBar");
        QScrollArea *scrollArea = Utils::findWidgetByAccessibleName<QScrollArea *>(m_settingDialog, "ContentScrollArea");
        if(titleBar && scrollArea) {
            qCDebug(mainprocess)<< "Branch: setting focus and scroll position";
            QTimer::singleShot(0, this, [titleBar, scrollArea](){
                titleBar->setFocus();
                scrollArea->verticalScrollBar()->setValue(0);
            });
        }
        return;
    }

    qCDebug(mainprocess)<< "Creating new settings dialog";
    QDateTime startTime = QDateTime::currentDateTime();
    // Warning: 此处虽然设置父控件，但是生命周期并不交由 pOwner 维护，因为终端存在多个同一层级的主窗体，
    //  每次调用 showSettingDialog() 时都会重新设置父窗口 setParent(pOwner, Flags)
    //  当前 pOwner 指向的主窗口销毁时，会优先调用 resetSettingOwner() 重设弹窗所有者，不会销毁 m_settingDialog 。
    //  m_settingDialog 弹窗在 Service::~Service() 析构时判断销毁。
    m_settingDialog = new DSettingsDialog(pOwner);
    m_settingDialog->setObjectName("SettingDialog");
    qCDebug(mainprocess)<< "Connecting dialog finished signal";
    connect(m_settingDialog, &DSettingsDialog::finished, this, &Service::slotSettingsDialogFinished);
    // 关闭时delete
    qCDebug(mainprocess)<< "Registering widget factory handlers";
    m_settingDialog->widgetFactory()->registerWidget("fontcombobox", Settings::createFontComBoBoxHandle);
    m_settingDialog->widgetFactory()->registerWidget("slider", Settings::createCustomSliderHandle);
    m_settingDialog->widgetFactory()->registerWidget("valslider", Settings::createValSliderHandle);
    m_settingDialog->widgetFactory()->registerWidget("spinbutton", Settings::createSpinButtonHandle);
    m_settingDialog->widgetFactory()->registerWidget("shortcut", Settings::createShortcutEditOptionHandle);
    m_settingDialog->widgetFactory()->registerWidget("tabformatedit", Settings::createTabTitleFormatOptionHandle);
    m_settingDialog->widgetFactory()->registerWidget("remotetabformatedit", Settings::createRemoteTabTitleFormatOptionHandle);
    m_settingDialog->widgetFactory()->registerWidget("shellconfigcombox", Settings::createShellConfigComboxOptionHandle);
    // 将数据重新读入
    qCDebug(mainprocess)<< "Updating settings";
    m_settingDialog->updateSettings(Settings::instance()->settings);
    // 设置窗口模态为没有模态，不阻塞窗口和进程
    qCDebug(mainprocess)<< "Setting window modality and moving to center";
    m_settingDialog->setWindowModality(Qt::NonModal);
    moveToCenter(m_settingDialog);
    QDateTime endTime = QDateTime::currentDateTime();

    //判断未开启窗口特效时，隐藏透明度/背景模糊选项
    showHideOpacityAndBlurOptions(DWindowManagerHelper::instance()->hasComposite());
    // 根据是否开启debuginfod选项初始化debuginfod urls选项的显示与否
    showHideDebuginfodUrlsOptions(Settings::instance()->enableDebuginfod());
}

void Service::slotSettingsDialogFinished(int result)
{
    qCDebug(mainprocess)<< "Enter Service::slotSettingsDialogFinished with result:" << result;
    Q_UNUSED(result)
    //激活设置框的有拥者
    if (m_settingOwner) {
        qCDebug(mainprocess)<< "Branch: activating setting owner window";
        m_settingOwner->activateWindow();
        m_settingOwner->focusCurrentPage();
    }
}

void Service::showHideOpacityAndBlurOptions(bool isShow)
{
    qCDebug(mainprocess)<< "Enter Service::showHideOpacityAndBlurOptions with isShow:" << isShow;
    if (nullptr == m_settingDialog) {
        qCWarning(mainprocess)  << "m_settingDialog is null";
        return;
    }

    qCDebug(mainprocess)<< "Looking for RightFrame widget";
    QWidget *rightFrame = m_settingDialog->findChild<QWidget *>("RightFrame");
    if (nullptr == rightFrame) {
        qCWarning(mainprocess)  << "can not found RightFrame in QWidget";
        return;
    }

    qCDebug(mainprocess)<< "Processing right frame widgets";
    QList<QWidget *> rightWidgetList = rightFrame->findChildren<QWidget *>();
    for (int i = 0; i < rightWidgetList.size(); i++) {
        // qCDebug(mainprocess)<< "Processing widget at index:" << i;
        QWidget *widget = rightWidgetList.at(i);
        if (nullptr == widget) {
            // qCDebug(mainprocess)<< "Branch: widget is null, continuing";
            continue;
        }

        if (strcmp(widget->metaObject()->className(), "QCheckBox") == 0) {
            // qCDebug(mainprocess)<< "Branch: found QCheckBox";
            QString checkText = (qobject_cast<QCheckBox *>(widget))->text();
            if (QObject::tr("Blur background") == checkText) {
                // qCDebug(mainprocess)<< "Branch: found blur background checkbox";
                QWidget *optionWidget = widget;
                QWidget *parentWidget = widget->parentWidget();
                if (parentWidget && strcmp(parentWidget->metaObject()->className(), "Dtk::Widget::DFrame") == 0) {
                    // qCDebug(mainprocess)<< "Branch: using parent DFrame widget";
                    optionWidget = parentWidget;
                }

                if (isShow) {
                    // qCDebug(mainprocess)<< "Branch: showing blur option";
                    optionWidget->show();
                } else {
                    // qCDebug(mainprocess)<< "Branch: hiding blur option";
                    optionWidget->hide();
                }
            }
        } else if (strcmp(widget->metaObject()->className(), "Dtk::Widget::DSlider") == 0) {
            // qCDebug(mainprocess)<< "Branch: found DSlider";
            QWidget *optionWidget = widget;
            QWidget *parentWidget = widget->parentWidget();
            if (parentWidget && strcmp(parentWidget->metaObject()->className(), "Dtk::Widget::DFrame") == 0) {
                // qCDebug(mainprocess)<< "Branch: using parent DFrame widget for slider";
                optionWidget = parentWidget;
            }

            if (isShow) {
                // qCDebug(mainprocess)<< "Branch: showing slider option";
                optionWidget->show();
            } else {
                // qCDebug(mainprocess)<< "Branch: hiding slider option";
                optionWidget->hide();
            }
        } else if (strcmp(widget->metaObject()->className(), "QLabel") == 0) {
            // qCDebug(mainprocess)<< "Branch: found QLabel";
            QString lblText = (qobject_cast<QLabel *>(widget))->text();
            if (lblText == QObject::tr("Opacity")) {
                // qCDebug(mainprocess)<< "Branch: found opacity label";
                QWidget *optionWidget = widget;
                QWidget *parentWidget = widget->parentWidget();
                if (parentWidget && strcmp(parentWidget->metaObject()->className(), "Dtk::Widget::DFrame") == 0) {
                    // qCDebug(mainprocess)<< "Branch: using parent DFrame widget for opacity label";
                    optionWidget = parentWidget;
                }

                if (isShow) {
                    // qCDebug(mainprocess)<< "Branch: showing opacity option";
                    optionWidget->show();
                } else {
                    // qCDebug(mainprocess)<< "Branch: hiding opacity option";
                    optionWidget->hide();
                }
            }
        } else {
            //do nothing
        }
    }
}

void Service::showHideDebuginfodUrlsOptions(bool isShow)
{
    QWidget *rightFrame = m_settingDialog->findChild<QWidget *>("RightFrame");
    if (nullptr == rightFrame) {
        qInfo() << "can not found RightFrame in QWidget";
        return;
    }

    QList<QWidget *> rightWidgetList = rightFrame->findChildren<QWidget *>();
    for (int i = 0; i < rightWidgetList.size(); i++) {
        QWidget *widget = rightWidgetList.at(i);
        if (nullptr == widget)
            continue;

        if (strcmp(widget->metaObject()->className(), "QLabel") == 0) {
            QString text = (qobject_cast<QLabel *>(widget))->text();
            if (QObject::tr("debuginfod URLs") == text) {
                QWidget *optionWidget = widget;
                QWidget *parentWidget = widget->parentWidget();
                QWidget *wrapWidget = parentWidget ? parentWidget->parentWidget() : nullptr;
                if (wrapWidget && strcmp(wrapWidget->metaObject()->className(), "QWidget") == 0) {
                    wrapWidget->setVisible(isShow);
                }

                if (parentWidget && strcmp(parentWidget->metaObject()->className(), "Dtk::Widget::DFrame") == 0)
                    optionWidget = parentWidget;

                if (isShow)
                    optionWidget->show();
                else
                    optionWidget->hide();
            }
        }
    }
}

void Service::listenWindowEffectSwitcher()
{
    qCDebug(mainprocess)<< "Enter Service::listenWindowEffectSwitcher";
    if (!m_mainTerminalIsInitWM) {
        qCDebug(mainprocess)<< "Branch: initializing window manager connection";
        connect(DWindowManagerHelper::instance(),&DWindowManagerHelper::hasBlurWindowChanged,this, &Service::slotWMChanged);
        m_mainTerminalIsInitWM = true;
    }
}

void Service::slotWMChanged()
{
    qCDebug(mainprocess)<< "Enter Service::slotWMChanged";
    bool isWinEffectEnabled = DWindowManagerHelper::instance()->hasBlurWindow();
    qCInfo(mainprocess) << "Window effect changed, blur enabled:" << isWinEffectEnabled;
    showHideOpacityAndBlurOptions(isWinEffectEnabled);
    qCDebug(mainprocess)<< "Emitting window effect enabled signal";
    emit onWindowEffectEnabled(isWinEffectEnabled);
}

bool Service::isWindowEffectEnabled()
{
    // qCDebug(mainprocess)<< "Enter Service::isWindowEffectEnabled";
    bool enabled = DWindowManagerHelper::instance()->hasBlurWindow();
    // qCDebug(mainprocess)<< "Window effect enabled:" << enabled;
    return enabled;
}

void Service::listenDebuginfodOption()
{
    connect(Settings::instance(), &Settings::terminalSettingChanged, this, [this] (const QString &keyName) {
        if ("advanced.debuginfod.enable_debuginfod" == keyName) {
            showHideDebuginfodUrlsOptions(Settings::instance()->enableDebuginfod());
        }
    });
}

qint64 Service::getEntryTime()
{
    // qCDebug(mainprocess)<< "Entry time:" << m_entryTime;
    return m_entryTime;
}

void Service::setScrollerTouchGesture(QAbstractScrollArea *widget)
{
    qCDebug(mainprocess)<< "Enter Service::setScrollerTouchGesture";
    QScroller::grabGesture(widget->viewport(), QScroller::TouchGesture);

    qCDebug(mainprocess)<< "Connecting scroller state changed signal";
    connect(QScroller::scroller(widget->viewport()), &QScroller::stateChanged, widget, [widget](QScroller::State newstate) {
        qCDebug(mainprocess)<< "Lambda: scroller state changed to:" << newstate;
        // fix bug#66335 触摸屏上滑动远程管理/自定义命令滚动条，列表滑动动画显示异常
        // 防止滑动时的鼠标事件导致viewport位置发生偏移。
        bool isDragging = (newstate == QScroller::Dragging);
        qCDebug(mainprocess)<< "Lambda: setting transparent for mouse events:" << isDragging;
        widget->viewport()->setAttribute(Qt::WA_TransparentForMouseEvents, isDragging);
    });
}


QMap<QString, QString> Service::getShells()
{
    qCDebug(mainprocess)<< "Enter Service::getShells";
    // 清空原有数据
    qCDebug(mainprocess)<< "Clearing existing shells map";
    m_shellsMap.clear();
    // 需要读取/etc/shells
    qCDebug(mainprocess)<< "Opening /etc/shells file";
    QFile shellsInfo(QStringLiteral("/etc/shells"));
    if (shellsInfo.open(QIODevice::ReadOnly)) {
        qCDebug(mainprocess)<< "Branch: shells file opened successfully";
        // 只读
        QTextStream stream(&shellsInfo);
        QString shellLine;
        // 循环读取
        do {
            QString shellPath;
            shellLine = stream.readLine();
            // 忽略注释
            if (!shellLine.startsWith(QLatin1String("#")) && !shellLine.isNull() && !shellLine.isEmpty()) {
                // qCDebug(mainprocess)<< "Branch: processing shell line:" << shellLine;
                // 获取shell所在目录
                shellPath = shellLine;
                // 获取shell进程名称
                QStringList shellPaths = shellPath.split(QLatin1String("/"));
                QString shellProgram = shellPaths.back();
                // 添加数据入map
                // qCDebug(mainprocess)<< "Adding shell to map:" << shellProgram << "->" << shellPath;
                m_shellsMap.insert(shellProgram, shellPath);
            }
        } while (!shellLine.isNull());
    } else {
        qCWarning(mainprocess)  << "read /etc/shells fail! error : " << shellsInfo.error();
    }
    // 关闭文件
    qCDebug(mainprocess)<< "Closing shells file";
    shellsInfo.close();
    qCDebug(mainprocess)<< "Found" << m_shellsMap.size() << "shells";
    return m_shellsMap;
}

QMap<QString, QString> Service::shellsMap()
{
    // qCDebug(mainprocess)<< "Enter Service::shellsMap";
    // qCDebug(mainprocess)<< "Returning shells map with" << m_shellsMap.size() << "entries";
    return m_shellsMap;
}

void Service::setMainTerminalIsStarted(bool started)
{
    // qCDebug(mainprocess)<< "Enter Service::setMainTerminalIsStarted with started:" << started;
    m_mainTerminalIsStarted = started;
}

bool Service::mainTerminalIsStarted()
{
    // qCDebug(mainprocess)<< "Enter Service::mainTerminalIsStarted";
    // qCDebug(mainprocess)<< "Main terminal started:" << m_mainTerminalIsStarted;
    return m_mainTerminalIsStarted;
}

void Service::showSettingDialog(MainWindow *pOwner)
{
    qCDebug(mainprocess) << "Showing settings dialog for owner:" << pOwner;
    // 第一次初始化dialog
    initSetting(pOwner);
    //保存设置框的有拥者
    m_settingOwner = pOwner;

    if (nullptr != m_settingDialog) {
        qCDebug(mainprocess) << "Branch: setting dialog is not null";
        //雷神需要让窗口置顶，可是普通窗口不要
        if (m_settingOwner == WindowsManager::instance()->getQuakeWindow()) {
            m_settingDialog->setWindowFlag(Qt::WindowStaysOnTopHint);
        } else {
            // 雷神窗口失去焦点自动隐藏
            if (WindowsManager::instance()->getQuakeWindow())
                WindowsManager::instance()->getQuakeWindow()->onAppFocusChangeForQuake();

            m_settingDialog->setWindowFlag(Qt::WindowStaysOnTopHint, false);
        }
        //更新设置的等宽字体
        Settings::instance()->handleWidthFont();
        FontFilter::instance()->handleWidthFont();

        // 重新加载shell配置数据
        qCDebug(mainprocess)<< "Reloading shell options";
        Settings::instance()->reloadShellOptions();

        // 重设当前弹窗的父窗口，在 DAbstractDialog / QDialog 的 show() / showEvent() 处理中，
        // 均含有关联父窗口的坐标计算，若不设置父控件，需单独计算显示坐标。setParent() 会重置Dialog标志，单独设置。
        if (pOwner != m_settingDialog->parentWidget()) {
            qCDebug(mainprocess)<< "Branch: setting new parent widget for dialog";
            m_settingDialog->setParent(pOwner, m_settingDialog->windowFlags() | Qt::Dialog);
        }
        qCDebug(mainprocess)<< "Showing settings dialog";
        m_settingDialog->show();
    } else {
        qCWarning(mainprocess)  << "No setting dialog.";
        return;
    }
    // 激活窗口
    qCDebug(mainprocess)<< "Activating settings dialog window";
    m_settingDialog->activateWindow();
}

void Service::hideSettingDialog()
{
    qCDebug(mainprocess)<< "Enter Service::hideSettingDialog";
    if (m_settingDialog) {
        qCDebug(mainprocess)<< "Branch: hiding settings dialog";
        m_settingDialog->hide();
    }
}

void Service::showCustomThemeSettingDialog(MainWindow *pOwner)
{
    qCDebug(mainprocess)<< "Enter Service::showCustomThemeSettingDialog with owner:" << pOwner;
    //保存设置框的有拥者
    m_settingOwner = pOwner;
    if (nullptr != m_customThemeSettingDialog) {
        qCDebug(mainprocess)<< "Branch: custom theme dialog already exists";
        //雷神需要让窗口置顶，可是普通窗口不要
        if (m_settingOwner == WindowsManager::instance()->getQuakeWindow()) {
            qCDebug(mainprocess)<< "Branch: setting window stays on top for quake window";
            m_customThemeSettingDialog->setWindowFlag(Qt::WindowStaysOnTopHint);
        } else {
            qCDebug(mainprocess)<< "Branch: removing window stays on top for normal window";
            // 雷神窗口失去焦点自动隐藏
            if (WindowsManager::instance()->getQuakeWindow()) {
                qCDebug(mainprocess)<< "Branch: handling quake window focus change";
                WindowsManager::instance()->getQuakeWindow()->onAppFocusChangeForQuake();
            }

            m_customThemeSettingDialog->setWindowFlag(Qt::WindowStaysOnTopHint, false);
        }
    } else {
        qCDebug(mainprocess)<< "Branch: creating new custom theme dialog";
        m_customThemeSettingDialog = new CustomThemeSettingDialog();

        qCDebug(mainprocess)<< "Connecting custom theme dialog finished signal";
        connect(m_customThemeSettingDialog, &CustomThemeSettingDialog::finished, this, &Service::slotCustomThemeSettingDialogFinished);
        // 设置窗口模态为没有模态，不阻塞窗口和进程
        m_customThemeSettingDialog->setWindowModality(Qt::NonModal);
        // 让设置与窗口等效，隐藏后显示就不会被遮挡
        m_customThemeSettingDialog->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
        qCDebug(mainprocess)<< "Moving custom theme dialog to center";
        moveToCenter(m_customThemeSettingDialog);
    }

    qCDebug(mainprocess)<< "Showing custom theme dialog";
    m_customThemeSettingDialog->show();

    // 激活窗口
    qCDebug(mainprocess)<< "Activating custom theme dialog window";
    m_customThemeSettingDialog->activateWindow();
}

void Service::slotCustomThemeSettingDialogFinished(int result)
{
    qCDebug(mainprocess)<< "Enter Service::slotCustomThemeSettingDialogFinished with result:" << result;
    if (CustomThemeSettingDialog::Accepted == result) {
        qCDebug(mainprocess)<< "Branch: dialog accepted, switching theme";
        m_settingOwner->switchThemeAction(m_settingOwner->themeCustomAction, Settings::instance()->m_configCustomThemePath);
        return;
    }
}

void Service::showShortcutConflictMsgbox(QString txt)
{
    qCDebug(mainprocess) << "Showing shortcut conflict message:" << txt;
    // 同步提示和快捷键
    for (QString key : ShortcutManager::instance()->m_mapReplaceText.keys()) {
        if (txt.contains(key)) {
            // qCDebug(mainprocess)<< "Branch: replacing" << key << "with" << ShortcutManager::instance()->m_mapReplaceText[key];
            txt.replace(key, ShortcutManager::instance()->m_mapReplaceText[key]);
        }
    }
    // 若没有设置弹框则退出，谈不上显示设置的快捷键冲突
    if (nullptr == m_settingDialog) {
        qCDebug(mainprocess)<< "Branch: no setting dialog, returning";
        return;
    }

    // 若没有弹窗，初始化
    if (nullptr == m_settingShortcutConflictDialog) {
        qCDebug(mainprocess)<< "Branch: creating new shortcut conflict dialog";
        m_settingShortcutConflictDialog = new DDialog(m_settingDialog);
        m_settingShortcutConflictDialog->setObjectName("ServiceSettingShortcutConflictDialog");
        m_settingShortcutConflictDialog->setIcon(QIcon::fromTheme("dialog-warning"));
        //将确认按钮设置为默认按钮
        m_settingShortcutConflictDialog->addButton(tr("OK", "button"), true, DDialog::ButtonNormal);

        qCDebug(mainprocess)<< "Connecting shortcut conflict dialog finished signal";
        connect(m_settingShortcutConflictDialog, &DDialog::finished, this, &Service::slotSettingShortcutConflictDialogFinished);
    }
    qCDebug(mainprocess)<< "Setting dialog title and showing";
    m_settingShortcutConflictDialog->setTitle(txt + QObject::tr("please set another one."));
    m_settingShortcutConflictDialog->show();
    // 将冲突窗口移到窗口中央
    qCDebug(mainprocess)<< "Moving conflict dialog to center";
    moveToCenter(m_settingShortcutConflictDialog);
}

void Service::slotSettingShortcutConflictDialogFinished()
{
    // qCDebug(mainprocess)<< "Enter Service::slotSettingShortcutConflictDialogFinished";
    // qCDebug(mainprocess)<< "Deleting shortcut conflict dialog";
    delete m_settingShortcutConflictDialog;
    m_settingShortcutConflictDialog = nullptr;
}

void Service::resetSettingOwner()
{
    qCDebug(mainprocess)<< "Enter Service::resetSettingOwner";
    m_settingOwner = nullptr;

    // m_settingDialog 生命周期不由父控件维护，在 Service 析构时销毁
    if (m_settingDialog) {
        qCDebug(mainprocess)<< "Branch: resetting setting dialog parent";
        m_settingDialog->setParent(nullptr);
    }
}

bool Service::isCountEnable()
{
    // qCDebug(mainprocess)<< "Enter Service::isCountEnable";
    bool enabled = WindowsManager::instance()->widgetCount() < MAXWIDGETCOUNT;
    // qCDebug(mainprocess)<< "Count enabled:" << enabled << "(" << WindowsManager::instance()->widgetCount() << "/" << MAXWIDGETCOUNT << ")";
    return enabled;
}

void Service::Entry(QStringList arguments)
{
    // qCDebug(mainprocess)<< "Enter Service::Entry with arguments:" << arguments;
    EntryTerminal(arguments, false);
}

void Service::EntryTerminal(QStringList arguments, bool isMain)
{
    qCDebug(mainprocess) << "Entering terminal with arguments:" << arguments;
    m_entryTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    TermProperties properties;
    Utils::parseCommandLine(arguments, properties);
    // 雷神处理入口
    if (properties[QuakeMode].toBool()) {
        qCDebug(mainprocess)<< "Branch: quake mode enabled, running quake window";
        WindowsManager::instance()->runQuakeWindow(properties);

        QJsonObject obj{
            {"tid", EventLogUtils::Start},
            {"version", QCoreApplication::applicationVersion()},
            {"mode", 1}
        };
        EventLogUtils::get().writeLogs(obj);

        return;
    }

    //首次启动的终端未启动
    if(!isMain && !mainTerminalIsStarted()) {
        qCDebug(mainprocess)<< "Branch: main terminal not started, returning";
        return;
    }
    // 超出最大窗口数量
    if(WindowsManager::instance()->widgetCount() >= MAXWIDGETCOUNT) {
        qCWarning(mainprocess)  << QString("terminal cannot be created: %1/%2 ")
                   .arg(WindowsManager::instance()->widgetCount())
                   .arg(MAXWIDGETCOUNT)
                   ;
        return;
    }
    qCDebug(mainprocess)<< "Creating normal window";
    WindowsManager::instance()->createNormalWindow(properties);

    QJsonObject obj{
        {"tid", EventLogUtils::Start},
        {"version", QCoreApplication::applicationVersion()},
        {"mode", 1}
    };
    EventLogUtils::get().writeLogs(obj);
}

void Service::onDesktopWorkspaceSwitched(int curDesktop, int nextDesktop)
{
    qCDebug(mainprocess) << "Enter Service::onDesktopWorkspaceSwitched with curDesktop:" << curDesktop << "nextDesktop:" << nextDesktop;
    Q_UNUSED(curDesktop)

    // 获取雷神窗口
    QuakeWindow *window = static_cast<QuakeWindow *>(WindowsManager::instance()->getQuakeWindow());
    // 没有雷神,直接返回
    if (nullptr == window) {
        qCDebug(mainprocess) << "Branch: quake window is null, returning";
        return;
    }
    // 雷神在所在桌面是否显示
    bool isQuakeVisible = window->isShowOnCurrentDesktop();
    // 判断下一个窗口是否是雷神所在的窗口
    if (nextDesktop != window->getDesktopIndex()) {
        qCDebug(mainprocess) << "Branch: next desktop is not current desktop, hiding quake window";
        // 下一个桌面不是当前桌面,隐藏
        if (isQuakeVisible) {
            // 根据雷神显隐,判断此时雷神的显隐,若已经隐了,不用再隐
            window->hide();
        }
        window->hide();
    } else {
        qCDebug(mainprocess) << "Branch: next desktop is current desktop, showing quake window";
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
    qCDebug(mainprocess) << "Service constructor";
    Utils::set_Object_Name(this);
    init();
}

bool Service::getIsDialogShow() const
{
    // qCDebug(mainprocess) << "Enter Service::getIsDialogShow";
    return m_isDialogShow;
}

void Service::setIsDialogShow(QWidget *parent, bool isDialogShow)
{
    qCDebug(mainprocess) << "Enter Service::setIsDialogShow with parent:" << parent << "isDialogShow:" << isDialogShow;
    MainWindow *window = qobject_cast<MainWindow *>(parent);
    if(nullptr == window) {
        qCDebug(mainprocess) << "Branch: window is null, returning";
        return;
    }
    if (window == WindowsManager::instance()->getQuakeWindow()) {
        qCInfo(mainprocess)  << "QuakeWindow show or hide dialog " << isDialogShow;
        m_isDialogShow = isDialogShow;
    }

    if (true == isDialogShow) {
        qCDebug(mainprocess) << "Branch: dialog show, disabling window";
        // 对话框显示,终端窗口禁用
        window->setEnabled(false);
    } else {
        qCDebug(mainprocess) << "Branch: dialog hide, enabling window";
        // 对话框隐藏或关闭,终端窗口启用,焦点重回终端
        window->setEnabled(true);
        window->focusCurrentPage();
    }
}

void Service::slotShowSettingsDialog()
{
    qCDebug(mainprocess) << "Enter Service::slotShowSettingsDialog";
    MainWindow *mainWindow = qobject_cast<MainWindow *>(sender()->parent());
    Service::instance()->showSettingDialog(mainWindow);
}
