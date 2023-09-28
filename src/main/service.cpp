// Copyright (C) 2019 ~ 2023 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
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
#include <QDBusConnection>
#include <QDBusConnectionInterface>

Service *Service::g_pService = nullptr;

Service *Service::instance()
{
    if (nullptr == g_pService) {
        g_pService = new Service();
    }
    return g_pService;
}

Service::~Service()
{
    if (nullptr != m_settingDialog) {
        delete m_settingDialog;
        m_settingDialog = nullptr;
    }
    if (nullptr != m_settingOwner) {
        delete m_settingOwner;
        m_settingOwner = nullptr;
    }
    if (nullptr != m_settingShortcutConflictDialog) {
        delete m_settingShortcutConflictDialog;
        m_settingShortcutConflictDialog = nullptr;
    }
    if (nullptr != m_customThemeSettingDialog) {
        delete m_customThemeSettingDialog;
        m_customThemeSettingDialog = nullptr;
    }
}

void Service::init()
{
    // 初始化自定义快捷键
    ShortcutManager::instance()->initShortcuts();
    // 初始化远程管理数据
    ServerConfigManager::instance()->initServerConfig();
    // 主进程：首次赋值m_pShareMemoryInfo
    listenWindowEffectSwitcher();
}

void Service::releaseInstance()
{
    if (nullptr != g_pService) {
        delete g_pService;
        g_pService = nullptr;
    }
}

void Service::initSetting()
{
    if (nullptr != m_settingDialog) {
        //1050e版本：二次打开设置窗口，焦点在【关闭按钮】上（bug#104810）
        DTitlebar *titleBar = Utils::findWidgetByAccessibleName<DTitlebar *>(m_settingDialog, "DSettingTitleBar");
        QScrollArea *scrollArea = Utils::findWidgetByAccessibleName<QScrollArea *>(m_settingDialog, "ContentScrollArea");
        if (titleBar && scrollArea) {
            QTimer::singleShot(0, this, [titleBar, scrollArea]() {
                titleBar->setFocus();
                scrollArea->verticalScrollBar()->setValue(0);
            });
        }
        return;
    }

    QDateTime startTime = QDateTime::currentDateTime();
    m_settingDialog = new DSettingsDialog();
    m_settingDialog->setObjectName("SettingDialog");
    // 关闭后将指针置空，下次重新new
    connect(m_settingDialog, &DSettingsDialog::finished, this, &Service::slotSettingsDialogFinished);
    // 关闭时delete
    m_settingDialog->widgetFactory()->registerWidget("fontcombobox", Settings::createFontComBoBoxHandle);
    m_settingDialog->widgetFactory()->registerWidget("slider", Settings::createCustomSliderHandle);
    m_settingDialog->widgetFactory()->registerWidget("valslider", Settings::createValSliderHandle);
    m_settingDialog->widgetFactory()->registerWidget("spinbutton", Settings::createSpinButtonHandle);
    m_settingDialog->widgetFactory()->registerWidget("shortcut", Settings::createShortcutEditOptionHandle);
    m_settingDialog->widgetFactory()->registerWidget("tabformatedit", Settings::createTabTitleFormatOptionHandle);
    m_settingDialog->widgetFactory()->registerWidget("remotetabformatedit", Settings::createRemoteTabTitleFormatOptionHandle);
    m_settingDialog->widgetFactory()->registerWidget("shellconfigcombox", Settings::createShellConfigComboxOptionHandle);
    // 将数据重新读入
    m_settingDialog->updateSettings(Settings::instance()->settings);
    // 设置窗口模态为没有模态，不阻塞窗口和进程
    m_settingDialog->setWindowModality(Qt::NonModal);
    moveToCenter(m_settingDialog);
    QDateTime endTime = QDateTime::currentDateTime();

    //判断未开启窗口特效时，隐藏透明度/背景模糊选项
    showHideOpacityAndBlurOptions(DWindowManagerHelper::instance()->hasComposite());
}

void Service::slotSettingsDialogFinished(int result)
{
    Q_UNUSED(result)
    //激活设置框的有拥者
    if (m_settingOwner) {
        m_settingOwner->activateWindow();
        m_settingOwner->focusCurrentPage();
    }
}

void Service::showHideOpacityAndBlurOptions(bool isShow)
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

        if (strcmp(widget->metaObject()->className(), "QCheckBox") == 0) {
            QString checkText = (qobject_cast<QCheckBox *>(widget))->text();
            if (QObject::tr("Blur background") == checkText) {
                QWidget *optionWidget = widget;
                QWidget *parentWidget = widget->parentWidget();
                if (parentWidget && strcmp(parentWidget->metaObject()->className(), "Dtk::Widget::DFrame") == 0)
                    optionWidget = parentWidget;

                if (isShow)
                    optionWidget->show();
                else
                    optionWidget->hide();
            }
        } else if (strcmp(widget->metaObject()->className(), "Dtk::Widget::DSlider") == 0) {
            QWidget *optionWidget = widget;
            QWidget *parentWidget = widget->parentWidget();
            if (parentWidget && strcmp(parentWidget->metaObject()->className(), "Dtk::Widget::DFrame") == 0)
                optionWidget = parentWidget;

            if (isShow)
                optionWidget->show();
            else
                optionWidget->hide();
        } else if (strcmp(widget->metaObject()->className(), "QLabel") == 0) {
            QString lblText = (qobject_cast<QLabel *>(widget))->text();
            if (lblText == QObject::tr("Opacity")) {
                QWidget *optionWidget = widget;
                QWidget *parentWidget = widget->parentWidget();
                if (parentWidget && strcmp(parentWidget->metaObject()->className(), "Dtk::Widget::DFrame") == 0)
                    optionWidget = parentWidget;

                if (isShow)
                    optionWidget->show();
                else
                    optionWidget->hide();
            }
        } else {
            //do nothing
        }
    }
}

void Service::listenWindowEffectSwitcher()
{
    connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasCompositeChanged, this, [this]() {
        bool isWinEffectEnabled = DWindowManagerHelper::instance()->hasComposite();
        showHideOpacityAndBlurOptions(isWinEffectEnabled);
        emit onWindowEffectEnabled(isWinEffectEnabled);
    });
}

qint64 Service::getEntryTime()
{
    return m_entryTime;
}

void Service::setScrollerTouchGesture(QAbstractScrollArea *widget)
{
    QScroller::grabGesture(widget->viewport(), QScroller::TouchGesture);

    connect(QScroller::scroller(widget->viewport()), &QScroller::stateChanged, widget, [widget](QScroller::State newstate) {
        // fix bug#66335 触摸屏上滑动远程管理/自定义命令滚动条，列表滑动动画显示异常
        // 防止滑动时的鼠标事件导致viewport位置发生偏移。
        bool isDragging = (newstate == QScroller::Dragging);
        widget->viewport()->setAttribute(Qt::WA_TransparentForMouseEvents, isDragging);
    });
}


QMap<QString, QString> Service::getShells()
{
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
            }
        } while (!shellLine.isNull());
    } else {
        qInfo() << "read /etc/shells fail! error : " << shellsInfo.error();
    }
    // 关闭文件
    shellsInfo.close();
    return m_shellsMap;
}

QMap<QString, QString> Service::shellsMap()
{
    return m_shellsMap;
}

void Service::setMainTerminalIsStarted(bool started)
{
    m_mainTerminalIsStarted = started;
}

bool Service::mainTerminalIsStarted()
{
    return m_mainTerminalIsStarted;
}

void Service::showSettingDialog(MainWindow *pOwner)
{
    // 第一次初始化dialog
    initSetting();
    //保存设置框的有拥者
    m_settingOwner = pOwner;
    if (nullptr != m_settingDialog) {
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

        // 重新加载shell配置数据
        Settings::instance()->reloadShellOptions();
        m_settingDialog->show();
    } else {
        qInfo() << "No setting dialog.";
        return;
    }
    // 激活窗口
    m_settingDialog->activateWindow();
}

void Service::hideSettingDialog()
{
    if (m_settingDialog)
        m_settingDialog->hide();
}

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
            if (WindowsManager::instance()->getQuakeWindow())
                WindowsManager::instance()->getQuakeWindow()->onAppFocusChangeForQuake();

            m_customThemeSettingDialog->setWindowFlag(Qt::WindowStaysOnTopHint, false);
        }
    } else {
        m_customThemeSettingDialog = new CustomThemeSettingDialog();

        connect(m_customThemeSettingDialog, &CustomThemeSettingDialog::finished, this, &Service::slotCustomThemeSettingDialogFinished);
        // 设置窗口模态为没有模态，不阻塞窗口和进程
        m_customThemeSettingDialog->setWindowModality(Qt::NonModal);
        // 让设置与窗口等效，隐藏后显示就不会被遮挡
        m_customThemeSettingDialog->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint);
        moveToCenter(m_customThemeSettingDialog);
    }

    m_customThemeSettingDialog->show();

    // 激活窗口
    m_customThemeSettingDialog->activateWindow();
}

void Service::slotCustomThemeSettingDialogFinished(int result)
{
    if (CustomThemeSettingDialog::Accepted == result) {
        m_settingOwner->customTheme(Settings::instance()->m_configCustomThemePath);
        return;
    }
}

void Service::showShortcutConflictMsgbox(QString txt)
{
    // 同步提示和快捷键
    for (QString key : ShortcutManager::instance()->m_mapReplaceText.keys()) {
        if (txt.contains(key))
            txt.replace(key, ShortcutManager::instance()->m_mapReplaceText[key]);
    }
    // 若没有设置弹框则退出，谈不上显示设置的快捷键冲突
    if (nullptr == m_settingDialog)
        return;

    // 若没有弹窗，初始化
    if (nullptr == m_settingShortcutConflictDialog) {
        m_settingShortcutConflictDialog = new DDialog(m_settingDialog);
        m_settingShortcutConflictDialog->setObjectName("ServiceSettingShortcutConflictDialog");
        m_settingShortcutConflictDialog->setIcon(QIcon::fromTheme("dialog-warning"));
        //将确认按钮设置为默认按钮
        m_settingShortcutConflictDialog->addButton(tr("OK", "button"), true, DDialog::ButtonNormal);

        connect(m_settingShortcutConflictDialog, &DDialog::finished, this, &Service::slotSettingShortcutConflictDialogFinished);
    }
    m_settingShortcutConflictDialog->setTitle(txt + QObject::tr("please set another one."));
    m_settingShortcutConflictDialog->show();
    // 将冲突窗口移到窗口中央
    moveToCenter(m_settingShortcutConflictDialog);
}

void Service::slotSettingShortcutConflictDialogFinished()
{
    delete m_settingShortcutConflictDialog;
    m_settingShortcutConflictDialog = nullptr;
}

bool Service::isCountEnable()
{
    return WindowsManager::instance()->widgetCount() < MAXWIDGETCOUNT;
}

void Service::Entry(QStringList arguments)
{
    EntryTerminal(arguments, false);
}

void Service::EntryTerminal(QStringList arguments, bool isMain)
{
    m_entryTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    TermProperties properties;
    Utils::parseCommandLine(arguments, properties);
    // 雷神处理入口
    if (properties[QuakeMode].toBool()) {
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
    if (!isMain && !mainTerminalIsStarted())
        return;
    // 超出最大窗口数量
    if (WindowsManager::instance()->widgetCount() >= MAXWIDGETCOUNT) {
        qInfo() << QString("terminal cannot be created: %1/%2 ")
                .arg(WindowsManager::instance()->widgetCount())
                .arg(MAXWIDGETCOUNT)
                ;
        return;
    }
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
    Q_UNUSED(curDesktop)

    // 获取雷神窗口
    QuakeWindow *window = static_cast<QuakeWindow *>(WindowsManager::instance()->getQuakeWindow());
    // 没有雷神,直接返回
    if (nullptr == window)
        return;
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
    init();
}

bool Service::getIsDialogShow() const
{
    return m_isDialogShow;
}

void Service::setIsDialogShow(QWidget *parent, bool isDialogShow)
{
    MainWindow *window = qobject_cast<MainWindow *>(parent);
    if (nullptr == window)
        return;
    if (window == WindowsManager::instance()->getQuakeWindow()) {
        qInfo() << "QuakeWindow show or hide dialog " << isDialogShow;
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
    MainWindow *mainWindow = qobject_cast<MainWindow *>(sender()->parent());
    Service::instance()->showSettingDialog(mainWindow);
}
