// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "termwidget.h"
#include "define.h"
#include "settings.h"
#include "termproperties.h"
#include "mainwindow.h"
#include "shortcutmanager.h"
#include "utils.h"
#include "service.h"
#include "windowsmanager.h"
#include "serverconfigmanager.h"

#include <DDesktopServices>
#include <DInputDialog>
#include <DGuiApplicationHelper>
#include <DLog>
#include <DDialog>
#include <DFloatingMessage>
#include <DMessageManager>
#include <DWindowManagerHelper>
#include <DSettingsOption>

#include <QApplication>
#include <QKeyEvent>
#include <QDesktopServices>
#include <QMenu>
#include <QVBoxLayout>
#include <QHostInfo>
#include <QTime>
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>
#include <QProcess>

DWIDGET_USE_NAMESPACE
using namespace Konsole;

#ifdef QT_DEBUG
Q_LOGGING_CATEGORY(views,"org.deepin.terminal.views")
#else
Q_LOGGING_CATEGORY(views,"org.deepin.terminal.views",QtInfoMsg)
#endif

TermWidget::TermWidget(const TermProperties &properties, QWidget *parent) : QTermWidget(0, parent), m_properties(properties)
{
    qCDebug(views) << "TermWidget constructor enter";
    Utils::set_Object_Name(this);
    // 窗口数量加1
    qCDebug(views) << "Increasing terminal count";
    WindowsManager::instance()->terminalCountIncrease();
    initTabTitle();
    m_page = static_cast<TermWidgetPage *>(parentWidget());
    setContextMenuPolicy(Qt::CustomContextMenu);

    qInfo() << "Setting initial history size:" << Settings::instance()->historySize();
    setHistorySize(Settings::instance()->historySize());
    setTerminalWordCharacters(Settings::instance()->wordCharacters());

    // 设置系统环境变量 - 这是必须的，确保shell能正确显示提示符和支持tab补全
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    
    // 设置debuginfod
    if (Settings::instance()->enableDebuginfod()) {
        if (!env.contains("DEBUGINFOD_URLS")) {
            env.insert("DEBUGINFOD_URLS", Settings::instance()->debuginfodUrls());
        }
    }
    
    // 将系统环境变量传递给终端，确保PS1、USER、HOME、PATH等重要变量可用
    setEnvironment(env.toStringList());

    QString strShellPath = Settings::instance()->shellPath();
    // set shell program
    qCInfo(views) << "set shell program : " << strShellPath;
    setShellProgram(strShellPath);
    /******** Modify by ut000610 daizhengwen 2020-07-08:初始化透明度 Begin***************/
    // 若没有窗口特效，则不生效
    // 若有窗口特效，生效

    // 此方法会丢失焦点（show hide方法丢失焦点）
    // setTermOpacity(Settings::instance()->opacity());

    qCDebug(views) << "Branch: Checking window effects and setting opacity";
    // 底层方法，设置当前窗口的透明度
    if (Service::instance()->isWindowEffectEnabled()) {
        qCDebug(views) << "Branch: Window effects enabled, setting opacity";
        // 判断当前是否有窗口特效
        setTerminalOpacity(Settings::instance()->opacity());
    }
    /********************* Modify by ut000610 daizhengwen End ************************/
    //setScrollBarPosition(QTermWidget::ScrollBarRight);//commend byq nyq

    /******** Modify by n014361 wangpeili 2020-01-13:              ****************/
    qCDebug(views) << "Branch: Setting up theme";
    // theme
    QString theme = "Dark";
   
    // 检查用户是否选择了特定主题
    QString expandThemeStr = Settings::instance()->extendColorScheme();
    if (!expandThemeStr.isEmpty()) {
        // 用户选择了自定义主题（Theme1, Theme2等），使用该主题
        qCDebug(views) << "Branch: Using user selected custom theme:" << expandThemeStr;
        setColorScheme(expandThemeStr, Settings::instance()->m_customThemeModify);
        Settings::instance()->m_customThemeModify = false;
    } else {
        // 用户选择了基础主题（浅色/深色）或"跟随系统"
        // 先检查用户是否选择了浅色或深色主题
        QString basicTheme = Settings::instance()->colorScheme();
        if (basicTheme == "Light" || basicTheme == "Dark") {
            // 用户明确选择了浅色或深色主题
            qCDebug(views) << "Branch: Using user selected basic theme:" << basicTheme;
            setColorScheme(basicTheme);
        } else {
            // 用户选择了"跟随系统"或初次启动，使用默认深色主题
            qCDebug(views) << "Branch: Using default Dark theme, system theme changes will be handled by theme change mechanism";
            setColorScheme(theme);
            Settings::instance()->setColorScheme(theme);
        }
    }

    // 这个参数启动为默认值UTF-8
    setTextCodec(QTextCodec::codecForName("UTF-8"));

    /******** Modify by n014361 wangpeili 2020-03-04: 增加保持打开参数控制，默认自动关闭**/
    /******* 2021-06-22  通过保持打开参数控制的值控制************/
    setAutoClose(!m_properties[KeepOpen].toBool());
    /********************* Modify by n014361 wangpeili End ************************/

    // WorkingDir
    if (m_properties.contains(WorkingDir))
        setWorkingDirectory(m_properties[WorkingDir].toString());

    if (m_properties.contains(Execute)) {
        //QString args = m_properties[Execute].toString();
        //QStringList argList = args.split(QRegExp(QStringLiteral("\\s+")), QString::SkipEmptyParts);
        QStringList argList = m_properties[Execute].toStringList();
        qCInfo(views) << "Execute args:" << argList;
        if (argList.count() > 0) {
            setShellProgram(argList.at(0));
            argList.removeAt(0);
            if (argList.count())
                setArgs(argList);
        }
    }

    // 字体和字体大小, 8#字体立即设置的时候会有BUG显示，做个延迟生效就好了。
    if (Settings::instance()->fontSize() == 8) {
        QTimer::singleShot(10, this, &TermWidget::onSetTerminalFont);
    } else {
        QFont font = getTerminalFont();
        font.setFamily(Settings::instance()->fontName());
        font.setPointSize(Settings::instance()->fontSize());
        setTerminalFont(font);
    }

    // 光标形状
    setKeyboardCursorShape(static_cast<QTermWidget::KeyboardCursorShape>(Settings::instance()->cursorShape()));
    // 光标闪烁
    setBlinkingCursor(Settings::instance()->cursorBlink());
    // 设置是否启用Ctrl+鼠标点击设置光标位置
    enableSetCursorPosition(Settings::instance()->enableSetCursorPosition());

    // 按键滚动
    setPressingScroll(Settings::instance()->PressingScroll());

    /******** Modify by ut000439 wangpeili 2020-07-27: fix bug 39371: 分屏线可以拉到边****/
    // 以最小mainwindow分4屏为标准的最小大小
    /******** Modify by ut001000 renfeixiang 2020-08-07:修改成根据全局变量m_MinWidth，m_MinHeight计算出term的最小高度和宽度***************/
    setMinimumSize(MainWindow::m_MinWidth / 2, (MainWindow::m_MinHeight - WIN_TITLE_BAR_HEIGHT) / 2);
    /********************* Modify by n014361 wangpeili End ************************/

    QString currentEnvLanguage = Utils::getCurrentEnvLanguage();
    // 判断是维吾尔语或者藏语时
    if (QStringLiteral("bo_CN") == currentEnvLanguage || QStringLiteral("ug_CN") == currentEnvLanguage) {
        // 启用双向文本渲染支持
        setBidiEnabled(true);
    }

    // fix bug#67979 Shell配置中设置无效shell程序名后，新增窗口无悬浮框提示
    initConnections();

    // 启动shell
    startShellProgram();

    // 增加可以自动运行脚本的命令，不需要的话，可以删除
    if (m_properties.contains(Script)) {
        QString args = m_properties[Script].toString();
        qCInfo(views) << "run cmd:" << args;
        args.append("\n");
        if (!m_properties[KeepOpen].toBool())
            args.append("exit\n");

        sendText(args);
    }

    setFocusPolicy(Qt::NoFocus);

    TermWidgetPage *parentPage = qobject_cast<TermWidgetPage *>(parent);
    connect(this, &QTermWidget::uninstallTerminal, parentPage, &TermWidgetPage::uninstallTerminal);
}

void TermWidget::initConnections()
{
    qCDebug(views) << "TermWidget::initConnections enter";
    // 输出滚动，会在每个输出判断是否设置了滚动，即时设置
    connect(this, &QTermWidget::receivedData, this, &TermWidget::onQTermWidgetReceivedData);

    // 接收到输出
    connect(this, &TermWidget::receivedData, this, &TermWidget::onTermWidgetReceivedData);

    // 接收到拖拽文件的Urls数据
    connect(this, &QTermWidget::sendUrlsToTerm, this, &TermWidget::onDropInUrls);

    connect(this, &QTermWidget::urlActivated, this, &TermWidget::onUrlActivated);

    connect(this, &QWidget::customContextMenuRequested, this, &TermWidget::customContextMenuCall);

    connect(DGuiApplicationHelper::instance(),
            &DGuiApplicationHelper::themeTypeChanged,
            this,
            &TermWidget::onThemeTypeChanged);

    // 未找到搜索的匹配结果
    connect(this, &QTermWidget::sig_noMatchFound, this, &TermWidget::onSig_noMatchFound);
    /********************* Modify by n014361 wangpeili End ************************/

    connect(this, &QTermWidget::isTermIdle, this, &TermWidget::onTermIsIdle);

    connect(this, &QTermWidget::shellWarningMessage, this, &TermWidget::onShellMessage);

    connect(this, &QTermWidget::titleChanged, this, &TermWidget::onTitleChanged);

    // 标题参数变化
    connect(this, &QTermWidget::titleArgsChange, this, &TermWidget::onTitleArgsChange);
    connect(this, &TermWidget::copyAvailable, this, &TermWidget::onCopyAvailable);

    connect(Settings::instance(), &Settings::terminalSettingChanged, this, &TermWidget::onSettingValueChanged);
    connect(Settings::instance(), &Settings::historySizeChanged, this, [this] (int newHistorySize) {
        qInfo() << "Setting new history size:" << newHistorySize;
        setHistorySize(newHistorySize);
    });

    //窗口特效开启则使用设置的透明度，窗口特效关闭时直接把窗口置为不透明
    connect(Service::instance(), &Service::onWindowEffectEnabled, this, &TermWidget::onWindowEffectEnabled);

    // 接收触控板事件
    connect(Service::instance(), &Service::touchPadEventSignal, this, &TermWidget::onTouchPadSignal);

    connect(Service::instance(), &Service::hostnameChanged, this, &TermWidget::onHostnameChanged);
}

inline void TermWidget::onSetTerminalFont()
{
    // qCDebug(views) << "Enter TermWidget::onSetTerminalFont";
    QFont font = getTerminalFont();
    font.setFamily(Settings::instance()->fontName());
    font.setPointSize(Settings::instance()->fontSize());
    setTerminalFont(font);
}

inline void TermWidget::onSig_noMatchFound()
{
    // qCDebug(views) << "Enter TermWidget::onSig_noMatchFound";
    parentPage()->setMismatchAlert(true);
}

inline void TermWidget::onQTermWidgetReceivedData(QString value)
{
    qCDebug(views) << "Enter TermWidget::onQTermWidgetReceivedData";
    Q_UNUSED(value)
    // 完善终端输出滚动相关功能，默认设置为"智能滚动"(即滚动条滑到最底下时自动滚动)
    if (!Settings::instance()->OutputtingScroll()) {
        qCDebug(views) << "Branch: OutputtingScroll is false, setting isAllowScroll to true";
        setIsAllowScroll(true);
        return;
    }

    // 获取是否允许输出时滚动
    if (getIsAllowScroll()) {
        qCDebug(views) << "Branch: getIsAllowScroll is true, setting trackOutput to OutputtingScroll";
        // 允许,则滚动到最新位置
        setTrackOutput(Settings::instance()->OutputtingScroll());
    } else {
        qCDebug(views) << "Branch: getIsAllowScroll is false, setting isAllowScroll to true";
        // 不允许,则不滚动
        // 将标志位置位
        setIsAllowScroll(true);
    }
}

inline void TermWidget::onTermWidgetReceivedData(QString value)
{
    qCDebug(views) << "Enter TermWidget::onTermWidgetReceivedData";
    //前提：
    //启动终端ForegroundPid:A
    //远程开始、输入信息中、远程中：ForegroundPid:B
    //远程结束ForegroundPid:A
    //远程开始时，快速ctrl+c，也会有ForegroundPid：A-》B-》A的过程

    //准备输入密码，且 ForegroundPid 不等于A时，为有效准备
    if(m_remotePasswordIsReady && getForegroundProcessId() != m_remoteMainPid) {
        qCDebug(views) << "Branch: m_remotePasswordIsReady is true and getForegroundProcessId is not equal to m_remoteMainPid";
        //匹配关键字
        if(value.toLower().contains("password:")
                || value.toLower().contains("enter passphrase for key")) {
            qCDebug(views) << "Branch: value contains password or passphrase";
        //输入密码,密码不为空，则发送
            if(!m_remotePassword.isEmpty())
                sendText(m_remotePassword + "\r");
            emit remotePasswordHasInputed();
        }
        //第一次远程时，需要授权
        if(value.toLower().contains("yes/no")) {
            qCDebug(views) << "Branch: value contains yes/no";
            sendText("yes\r");
        }

    }
    //若ForegroundPid等于A，则代表远程结束，如开始连接时立刻ctrl+c
    if(m_remotePasswordIsReady && getForegroundProcessId() == m_remoteMainPid) {
        qCDebug(views) << "Branch: m_remotePasswordIsReady is true and getForegroundProcessId is equal to m_remoteMainPid";
        m_remotePasswordIsReady = false;
    }

    /******** Modify by ut000610 daizhengwen 2020-05-25: quit download****************/
    if (value.contains("Transfer incomplete")) {
        qCDebug(views) << "Branch: value contains Transfer incomplete";
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_C, Qt::ControlModifier);
        QApplication::sendEvent(focusWidget(), &keyPress);
    }
    if (value.endsWith("\b \b #")) {                     // 结束的时候有乱码的话，将它清除
        qCDebug(views) << "Branch: value ends with \b \b #";
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_U, Qt::ControlModifier);
        QApplication::sendEvent(focusWidget(), &keyPress);
    }
    /********************* Modify by ut000610 daizhengwen End ************************/
    // 退出远程后，设置成false
    if ((value.contains("Connection to") && value.contains(" closed.")) || value.contains("Permission denied"))
        QTimer::singleShot(100, this, &TermWidget::onExitRemoteServer);
}

inline void TermWidget::onExitRemoteServer()
{
    qCDebug(views) << "TermWidget::onExitRemoteServer - Checking remote status";
    // 判断是否此时退出远程
    if (!isInRemoteServer()) {
        qCInfo(views) << "exit remote";
        qCDebug(views) << "TermWidget::onExitRemoteServer - Exiting remote server";
        setIsConnectRemote(false);
        // 还原编码
        setTextCodec(QTextCodec::codecForName(encode().toLocal8Bit()));
        qCInfo(views) << "current encode " << encode();
        qCDebug(views) << "TermWidget::onExitRemoteServer - Restoring original settings";
        setBackspaceMode(m_backspaceMode);
        setDeleteMode(m_deleteMode);
        emit Service::instance()->checkEncode(encode());
    } else {
        qCDebug(views) << "TermWidget::onExitRemoteServer - Still in remote server";
    }
}

inline void TermWidget::onUrlActivated(const QUrl &url, bool fromContextMenu)
{
    qCDebug(views) << "Enter TermWidget::onUrlActivated";
    if (QApplication::keyboardModifiers() & Qt::ControlModifier || fromContextMenu)
        QDesktopServices::openUrl(url);
}

inline void TermWidget::onThemeTypeChanged(DGuiApplicationHelper::ColorType builtInTheme)
{
    qCInfo(views) << "Theme Type Changed! Current Theme Type: " << builtInTheme;
    qCDebug(views) << "TermWidget::onThemeTypeChanged - Start theme update";
    // ThemePanelPlugin *plugin = qobject_cast<ThemePanelPlugin *>(getPluginByName("Theme"));
    QString theme = "Dark";
    /************************ Mod by sunchengxi 2020-09-16:Bug#48226#48230#48236#48241 终端默认主题色应改为深色修改引起的系列问题修复 Begin************************/
    //Mod by sunchengxi 2020-09-17:Bug#48349 主题色选择跟随系统异常
    if (builtInTheme == DGuiApplicationHelper::LightType)
        theme = "Light";

    /************************ Mod by sunchengxi 2020-09-16:Bug#48226#48230#48236#48241 终端默认主题色应改为深色修改引起的系列问题修复 End ************************/
    //setColorScheme(theme);
    //Settings::instance()->setColorScheme(theme);
    QString  expandThemeStr = "";
    expandThemeStr = Settings::instance()->extendColorScheme();
    if (expandThemeStr.isEmpty()) {
        if (DGuiApplicationHelper::instance()->paletteType() == DGuiApplicationHelper::LightType)
            theme = "Light";

        setColorScheme(theme);
        Settings::instance()->setColorScheme(theme);
    } else {
        setColorScheme(expandThemeStr, Settings::instance()->m_customThemeModify);
        Settings::instance()->m_customThemeModify = false;
    }
}

inline void TermWidget::onTermIsIdle(bool bIdle)
{
    qCDebug(views) << "Enter TermWidget::onTermIsIdle";
    emit termIsIdle(m_page->identifier(), bIdle);
}

inline void TermWidget::onTitleChanged()
{
    qCDebug(views) << "Enter TermWidget::onTitleChanged";
    // 解析shell传来的title 用户名 主机名 地址/目录
    QString tabTitle = TermWidget::title();
    // %w shell设置的窗口标题
    m_tabArgs[SHELL_TITLE] = tabTitle;
    m_remoteTabArgs[SHELL_TITLE] = tabTitle;
    // 将标题参数变更信息传出
    emit termTitleChanged(getTabTitle());
}

inline void TermWidget::onCopyAvailable(bool enable)
{
    qCDebug(views) << "Enter TermWidget::onCopyAvailable";
    if (Settings::instance()->IsPasteSelection() && enable) {
        qCDebug(views) << "Branch: IsPasteSelection is true and enable is true";
        QString strSelected = selectedText();
        QApplication::clipboard()->setText(strSelected, QClipboard::Clipboard);
    }
}

inline void TermWidget::onWindowEffectEnabled(bool isWinEffectEnabled)
{
    qCDebug(views) << "TermWidget::onWindowEffectEnabled - Window effect:" << isWinEffectEnabled;
    if (isWinEffectEnabled) {
        qCDebug(views) << "Setting terminal opacity to:" << Settings::instance()->opacity();
        this->setTermOpacity(Settings::instance()->opacity());
    } else {
        qCDebug(views) << "Setting terminal opacity to full (1.0)";
        this->setTermOpacity(1.0);
    }
}

TermWidget::~TermWidget()
{
    // qCDebug(views) << "Enter TermWidget::~TermWidget";
    // 窗口减1
    WindowsManager::instance()->terminalCountReduce();
}

TermWidgetPage *TermWidget::parentPage()
{
    // qCDebug(views) << "Enter TermWidget::parentPage";
    return  m_page;
}

void TermWidget::onTitleArgsChange(QString key, QString value)
{
    qCDebug(views) << "Enter TermWidget::onTitleArgsChange";
    // tab获取参数
    m_tabArgs[key] = value;
    // 获取当前目录短 由长到短
    if (DIR_L == key) {
        qCDebug(views) << "Branch: DIR_L is equal to key";
        QString dir = value;
        // 当前目录短（若有优化方法请及时告知）
        if ("~" == dir || dir.startsWith(QDir::homePath())) {
            // 出现家目录~的情况
            QString homePath = QDir::homePath();
            QStringList pathList;
            if("~" == dir)
                pathList = homePath.split("/");
            else
                pathList = dir.split("/");
            // 当前目录短，返回当前文件夹名
            m_tabArgs[DIR_S] = pathList.value(pathList.count() - 1);
            // 当前目录长对于~的处理 => 传过来的不是~但要填进去~和提示符保持一致
            dir.replace(homePath, "~");
            m_tabArgs[DIR_L] = dir;
        } else if ("/" == dir) {
            qCDebug(views) << "Branch: / is equal to dir";
            // 出现根目录/的情况
            m_tabArgs[DIR_S] = "/";
        } else {
            qCDebug(views) << "Branch: dir is not ~ or /";
            // 一般情况
            QStringList pathList = dir.split("/");
            m_tabArgs[DIR_S] = pathList.last();
        }
    }

    // 更改标签标题
    emit termTitleChanged(getTabTitle());
}

void TermWidget::onHostnameChanged()
{
    qCDebug(views) << "Enter TermWidget::onHostnameChanged";
    // 主机名变化
    QString hostName = QHostInfo::localHostName();
    m_tabArgs[LOCAL_HOST_NAME] = hostName;
    // 发送标签标题变化
    emit termTitleChanged(getTabTitle());
}

inline void TermWidget::onCopy()
{
    qCDebug(views) << "Enter TermWidget::onCopy";
    copyClipboard();
}

inline void TermWidget::onPaste()
{
    qCDebug(views) << "Enter TermWidget::onPaste";
    pasteClipboard();
}

inline void TermWidget::onOpenFileInFileManager()
{
    qCDebug(views) << "Enter TermWidget::onOpenFileInFileManager";
    //DDesktopServices::showFolder(QUrl::fromLocalFile(workingDirectory()));

    //打开文件夹的方式 和  打开文件夹 并勾选文件的方式 如下
    //dde-file-manager -n /data/home/lx777/my-wjj/git/2020-08/18-zoudu/build-deepin-terminal-unknown-Debug
    //dde-file-manager --show-item a.pdf

    QProcess process;
    //未选择内容
    if (selectedText().isEmpty()) {
        qCDebug(views) << "Branch: selectedText is empty, starting dde-file-manager with workingDirectory";
        process.startDetached("dde-file-manager", {"-n", workingDirectory()});
        return;
    }

    QFileInfo fi(workingDirectory() + "/" + selectedText());
    //选择的内容是文件或者文件夹
    if (fi.isFile() || fi.isDir()) {
        qCDebug(views) << "Branch: selectedText is a file or directory, starting dde-file-manager with --show-item";
        process.startDetached("dde-file-manager", {"--show-item", workingDirectory() + "/" + selectedText()});
        return;
    }
    //选择的文本不是文件也不是文件夹
    process.startDetached("dde-file-manager", {"-n", workingDirectory()});
}

/*** 修复 bug 28162 鼠标左右键一起按终端会退出 ***/
void TermWidget::addMenuActions(const QPoint &pos)
{
    qCDebug(views) << "Enter TermWidget::addMenuActions";
    bool isRemoting = isConnectRemote();

    QList<QAction *> termActions = filterActions(pos);
    for (QAction *&action : termActions)
        m_menu->addAction(action);

    if (!m_menu->isEmpty())
        m_menu->addSeparator();

    // add other actions here.
    if (!selectedText().isEmpty())
        m_menu->addAction(tr("Copy"), this, &TermWidget::onCopy);

    if (!QApplication::clipboard()->text(QClipboard::Clipboard).isEmpty())
        m_menu->addAction(tr("Paste"), this, &TermWidget::onPaste);

    /******** Modify by n014361 wangpeili 2020-02-26: 添加打开(文件)菜单功能 **********/
    if (!isRemoting && !selectedText().isEmpty()) {
        QString fileName = getFormatFileName(selectedText());
        QString filePath = getFilePath(fileName);
        QUrl fileUrl = QUrl::fromLocalFile(filePath);
        QFileInfo tempfile(fileUrl.path());
        if (fileName.length() > 0 && tempfile.exists())
            m_menu->addAction(tr("Open"), this, &TermWidget::onOpenFile);
    }
    /********************* Modify by n014361 wangpeili End ************************/

    m_menu->addAction(tr("Open in file manager"), this, &TermWidget::onOpenFileInFileManager);

    m_menu->addSeparator();


    DSplitter *splitter = qobject_cast<DSplitter *>(parentWidget());
    int layer = getTermLayer();

    if (1 == layer || (2 == layer && splitter && Qt::Horizontal == splitter->orientation()))
        m_menu->addAction(tr("Horizontal split"), this, &TermWidget::onHorizontalSplit);

    if (1 == layer || (2 == layer && splitter && Qt::Vertical == splitter->orientation()))
        m_menu->addAction(tr("Vertical split"), this, &TermWidget::onVerticalSplit);

    /******** Modify by n014361 wangpeili 2020-02-21: 增加关闭窗口和关闭其它窗口菜单    ****************/
    m_menu->addAction(QObject::tr("Close workspace"), this, &TermWidget::onCloseCurrWorkSpace);
    //m_menu->addAction(tr("Close Window"), this, [this] { ((TermWidgetPage *)m_Page)->close();});
    if (parentPage()->getTerminalCount() > 1)
        m_menu->addAction(QObject::tr("Close other workspaces"), this, &TermWidget::onCloseOtherWorkSpaces);

    /********************* Modify by n014361 wangpeili End ************************/
    m_menu->addSeparator();
    m_menu->addAction(tr("New tab"), this, &TermWidget::onCreateNewTab);

    m_menu->addSeparator();

    if (!parentPage()->parentMainWindow()->isQuakeMode()) {
        bool isFullScreen = this->window()->windowState().testFlag(Qt::WindowFullScreen);
        if (isFullScreen)
            m_menu->addAction(tr("Exit fullscreen"), this, &TermWidget::onSwitchFullScreen);
        else
            m_menu->addAction(tr("Fullscreen"), this, &TermWidget::onSwitchFullScreen);
    }

    m_menu->addAction(tr("Find"), this, &TermWidget::onShowSearchBar);
    m_menu->addSeparator();

    if (!selectedText().isEmpty()) {
        DMenu *search = new DMenu(tr("Search"), this);

        search->addAction("Bing", this, &TermWidget::openBing);
        search->addAction("Baidu", this, &TermWidget::openBaidu);
        search->addAction("Github", this, &TermWidget::openGithub);
        search->addAction("Stack Overflow", this, &TermWidget::openStackOverflow);
        m_menu->addMenu(search);
        m_menu->addSeparator();
    }

    m_menu->addAction(tr("Encoding"), this, &TermWidget::onShowEncoding);

    m_menu->addAction(tr("Custom commands"), this, &TermWidget::onShowCustomCommands);

    m_menu->addAction(tr("Remote management"), this, &TermWidget::onShowRemoteManagement);

    if (isConnectRemote()) {
        m_menu->addSeparator();
        m_menu->addAction(tr("Upload file"), this, &TermWidget::onUploadFile);
        m_menu->addAction(tr("Download file"), this, &TermWidget::onDownloadFile);
    }

    m_menu->addSeparator();

    m_menu->addAction(tr("Settings"), this, &TermWidget::onShowSettings);

    // 判断是否到达最大数量
    if (!Service::instance()->isCountEnable()) {
        QList<QAction *> actionList = m_menu->actions();
        for (auto item : actionList) {
            if (item->text() == tr("New tab") || item->text() == tr("Horizontal split") || item->text() == tr("Vertical split")) {
                // 无法点击的菜单项置灰
                item->setEnabled(false);
            }
        }
    }
}

inline void TermWidget::onHorizontalSplit()
{
    qCDebug(views) << "Enter TermWidget::onHorizontalSplit";
    getTermLayer();
    // menu关闭与分屏同时进行时，会导致QT计算光标位置异常。
    QTimer::singleShot(10, this, &TermWidget::splitHorizontal);
}

inline void TermWidget::onVerticalSplit()
{
    qCDebug(views) << "Enter TermWidget::onVerticalSplit";
    getTermLayer();
    // menu关闭与分屏同时进行时，会导致QT计算光标位置异常。
    QTimer::singleShot(10, this, &TermWidget::splitVertical);
}

inline void TermWidget::splitHorizontal()
{
    // qCDebug(views) << "Enter TermWidget::splitHorizontal";
    parentPage()->split(Qt::Horizontal);
}

inline void TermWidget::splitVertical()
{
    // qCDebug(views) << "Enter TermWidget::splitVertical";
    parentPage()->split(Qt::Vertical);
}

inline void TermWidget::onCloseCurrWorkSpace()
{
    qCDebug(views) << "Enter TermWidget::onCloseCurrWorkSpace";
    parentPage()->closeSplit(parentPage()->currentTerminal());
}

inline void TermWidget::onCloseOtherWorkSpaces()
{
    qCDebug(views) << "Enter TermWidget::onCloseOtherWorkSpaces";
    parentPage()->closeOtherTerminal();
}

inline void TermWidget::onCreateNewTab()
{
    qCDebug(views) << "Enter TermWidget::onCreateNewTab";
    parentPage()->parentMainWindow()->createNewTab();
}

inline void TermWidget::onSwitchFullScreen()
{
    qCDebug(views) << "Enter TermWidget::onSwitchFullScreen";
    parentPage()->parentMainWindow()->switchFullscreen();
}

inline void TermWidget::openBing()
{
    qCDebug(views) << "Enter TermWidget::openBing";
    QString strUrl = "https://cn.bing.com/search?q=" + selectedText();
    openUrl(strUrl);
}

inline void TermWidget::openBaidu()
{
    qCDebug(views) << "Enter TermWidget::openBaidu";
    QString strUrl = "https://www.baidu.com/s?wd=" + selectedText();
    openUrl(strUrl);
}

inline void TermWidget::openGithub()
{
    qCDebug(views) << "Enter TermWidget::openGithub";
    QString strUrl = "https://github.com/search?q=" + selectedText();
    openUrl(strUrl);
}

inline void TermWidget::openStackOverflow()
{
    qCDebug(views) << "Enter TermWidget::openStackOverflow";
    QString strUrl = "https://stackoverflow.com/search?q=" + selectedText();
    openUrl(strUrl);
}

inline void TermWidget::onShowSearchBar()
{
    qCDebug(views) << "Enter TermWidget::onShowSearchBar";
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_SEARCHBAR);
}

inline void TermWidget::onShowEncoding()
{
    qCDebug(views) << "Enter TermWidget::onShowEncoding";
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);
}

inline void TermWidget::onShowCustomCommands()
{
    qCDebug(views) << "Enter TermWidget::onShowCustomCommands";
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);
}

inline void TermWidget::onShowRemoteManagement()
{
    qCDebug(views) << "Enter TermWidget::onShowRemoteManagement";
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);
}

inline void TermWidget::onUploadFile()
{
    qCDebug(views) << "Enter TermWidget::onUploadFile";
    parentPage()->parentMainWindow()->remoteUploadFile();
}

inline void TermWidget::onDownloadFile()
{
    qCDebug(views) << "Enter TermWidget::onDownloadFile";
    parentPage()->parentMainWindow()->remoteDownloadFile();
}

inline void TermWidget::onShowSettings()
{
    qCDebug(views) << "Enter TermWidget::onShowSettings";
    Service::instance()->showSettingDialog(parentPage()->parentMainWindow());
}

inline void TermWidget::openUrl(QString strUrl)
{
    qCDebug(views) << "Enter TermWidget::openUrl:" << strUrl;
    QDesktopServices::openUrl(QUrl(strUrl));
}

inline QString TermWidget::getFormatFileName(QString selectedText)
{
    qCDebug(views) << "Enter TermWidget::getFormatFileName:" << selectedText;
    QString fileName = selectedText.trimmed();
    if ((fileName.startsWith("'") && fileName.endsWith("'"))
            || (fileName.startsWith("\"") && fileName.endsWith("\""))) {
        qCDebug(views) << "Branch: fileName is wrapped in quotes";
        fileName = fileName.remove(0, 1);
        fileName = fileName.remove(fileName.length() - 1, 1);
        qCInfo(views) << "Format fileName is " << fileName;
    }

    return fileName;
}

inline QString TermWidget::getFilePath(QString fileName)
{
    qCDebug(views) << "Enter TermWidget::getFilePath:" << fileName;
    //如果fileName本身已经是一个文件路径
    if (fileName.startsWith("/")) {
        qCDebug(views) << "Branch: fileName starts with /, returning fileName";
        return fileName;
    }

    return workingDirectory() + "/" + fileName;
}

void TermWidget::inputRemotePassword(const QString &remotePassword)
{
    qCDebug(views) << "Enter TermWidget::inputRemotePassword";
    //每个工作区设置对应的标志位
    m_remoteMainPid = getForegroundProcessId();
    m_remotePassword = remotePassword;
    m_remotePasswordIsReady = true;

    //等待密码输入或超时，超时暂不处理
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(this, &TermWidget::remotePasswordHasInputed, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(1000);
    loop.exec();

    //还原
    m_remotePassword = "";
    m_remotePasswordIsReady = false;
}

inline void TermWidget::onOpenFile()
{
    qCDebug(views) << "Enter TermWidget::onOpenFile";
    QString fileName = getFormatFileName(selectedText());
    QString filePath = getFilePath(fileName);
    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    QDesktopServices::openUrl(fileUrl);
}

QString TermWidget::RemoteEncode() const
{
    // qCDebug(views) << "Enter TermWidget::RemoteEncode";
    return m_remoteEncode;
}

void TermWidget::setRemoteEncode(const QString &RemoteEncode)
{
    // qCDebug(views) << "Enter TermWidget::setRemoteEncode:" << RemoteEncode;
    m_remoteEncode = RemoteEncode;
}

void TermWidget::setBackspaceMode(const EraseMode &backspaceMode)
{
    qCDebug(views) << "Enter TermWidget::setBackspaceMode:" << backspaceMode;
    char ch;
    int length;
    switch (backspaceMode) {
    case EraseMode_Auto:
        ch = getErase();
        if (ch == 0)
            ch = '\010';
        length = 1;
        break;
    case EraseMode_Control_H:
        ch = '\010';
        length = 1;
        break;
    case EraseMode_Ascii_Delete:
        ch = '\177';
        length = 1;
        break;
    case EraseMode_TTY:
        ch = getErase();
        length = 1;
        qCInfo(views) << "tty erase : " << QByteArray(&ch, length).toHex();
        break;
    case EraseMode_Escape_Sequeue:
        length = 4;
        char sequeue[] = "\e[3~";
        QTermWidget::setBackspaceMode(sequeue, 4);
        return;
    }
    QTermWidget::setBackspaceMode(&ch, length);
}

void TermWidget::setDeleteMode(const EraseMode &deleteMode)
{
    qCDebug(views) << "Enter TermWidget::setDeleteMode:" << deleteMode;
    char ch;
    int length;
    switch (deleteMode) {
    case EraseMode_Control_H:
        ch = '\010';
        length = 1;
        break;
    case EraseMode_Ascii_Delete:
        ch = '\177';
        length = 1;
        break;
    case EraseMode_TTY:
        ch = getErase();
        length = 1;
        qCInfo(views) << "tty erase : " << QByteArray(&ch, length).toHex();
        break;
    case EraseMode_Auto:
    case EraseMode_Escape_Sequeue:
        length = 4;
        char sequeue[] = "\e[3~";
        QTermWidget::setDeleteMode(sequeue, 4);
        return;
    }
    QTermWidget::setDeleteMode(&ch, length);
}

int TermWidget::getTermLayer()
{
    qCDebug(views) << "Enter TermWidget::getTermLayer";
    int layer = 1;
    QWidget *currentW = this;
    while (currentW->parentWidget() != parentPage()) {
        layer++;
        currentW = currentW->parentWidget();
    }
    return  layer;
}

void TermWidget::setTabFormat(const QString &tabFormat)
{
    qCDebug(views) << "Enter TermWidget::setTabFormat:" << tabFormat;
    // 非全局设置优先级更高
    if (m_tabFormat.isGlobal != false && m_tabFormat.currentTabFormat != tabFormat) {
        m_tabFormat.currentTabFormat = tabFormat;
        // 有变化，就将变化发出，但此时可能显示的不是变化的项
        emit termTitleChanged(getTabTitle());
    }
}

void TermWidget::setRemoteTabFormat(const QString &remoteTabFormat)
{
    qCDebug(views) << "Enter TermWidget::setRemoteTabFormat:" << remoteTabFormat;
    // 非全局设置优先级更高
    if (m_tabFormat.isGlobal != false && m_tabFormat.remoteTabFormat != remoteTabFormat) {
        m_tabFormat.remoteTabFormat = remoteTabFormat;
        // 有变化，就将变化发出，但此时可能显示的不是变化的项
        emit termTitleChanged(getTabTitle());
    }
}

void TermWidget::renameTabFormat(const QString &tabFormat, const QString &remoteTabFormat)
{
    qCDebug(views) << "Enter TermWidget::renameTabFormat:" << tabFormat << remoteTabFormat;
    // 重命名优先级高
    m_tabFormat.currentTabFormat = tabFormat;
    m_tabFormat.remoteTabFormat = remoteTabFormat;
    m_tabFormat.isGlobal = false;
    emit termTitleChanged(getTabTitle());
}

QString TermWidget::getTabTitle()
{
    qCDebug(views) << "Enter TermWidget::getTabTitle";
    QString strTabName;
    // 判断当前是否连接远程
    if (isConnectRemote() /*|| getForegroundProcessName() == "ssh"*/) {
        // 连接远程 远程标签标题格式
        strTabName = getTabTitle(m_remoteTabArgs, m_tabFormat.remoteTabFormat);
    } else {
        // 当前标签标题格式
        strTabName = getTabTitle(m_tabArgs, m_tabFormat.currentTabFormat);
    }

    // 没有内容则给Terminal作为默认标题
    if (strTabName.trimmed().isEmpty())
        strTabName = DEFAULT_TAB_TITLE;

    return strTabName;
}

QString TermWidget::getTabTitle(QMap<QString, QString> format, QString TabFormat)
{
    qCDebug(views) << "Enter TermWidget::getTabTitle";
    // 遍历参数列表
    for (QString key : format.keys()) {
        // 判断是否包含改参数
        if (TabFormat.contains(key)) {
            // 包含的话，把参数替换为参数对应的内容
            TabFormat.replace(key, format[key]);
        }
    }
    return TabFormat;
}

void TermWidget::initTabTitle()
{
    qCDebug(views) << "Enter TermWidget::initTabTitle";
    static ushort sessionNumber = 0;
    // 初始化标签标题参数
    initTabTitleArgs();
    // 会话编号赋值
    m_sessionNumber = ++sessionNumber;
    m_tabArgs[TAB_NUM] = QString::number(m_sessionNumber);
    m_remoteTabArgs[TAB_NUM] = QString::number(m_sessionNumber);
    // 初始化hostName
    m_tabArgs[LOCAL_HOST_NAME] = QHostInfo::localHostName();
    // 设置标签标题格式
    setTabFormat(Settings::instance()->tabTitleFormat());
    setRemoteTabFormat(Settings::instance()->remoteTabTitleFormat());
    // 标签标题变化，则每个term变化
    connect(Settings::instance(), &Settings::tabFormatChanged, this, &TermWidget::setTabFormat);
    connect(Settings::instance(), &Settings::remoteTabFormatChanged, this, &TermWidget::setRemoteTabFormat);
    // TODO: Fix this signal connection - enableSetCursorPosition signal not properly implemented
    // connect(Settings::instance(), &Settings::enableSetCursorPosition, this, [this](bool enable) {
    //     enableSetCursorPosition(enable);
    // });
}

void TermWidget::initTabTitleArgs()
{
    qCDebug(views) << "Enter TermWidget::initTabTitleArgs";
    QStringList strTabArgs = TAB_ARGS.split(" ");
    // 填充标签标题参数
    for (QString arg : strTabArgs)
        m_tabArgs.insert(arg, " ");

    // 填充远程标题标签参数
    QStringList strRemoteTabArgs = REMOTE_ARGS.split(" ");
    for (QString arg : strRemoteTabArgs)
        m_remoteTabArgs.insert(arg, " ");

    qCInfo(views) << "Tab args init! tab title count : " << m_tabArgs.count() << " remote title count : " << m_remoteTabArgs.count();
}

QString TermWidget::getTabTitleFormat()
{
    // qCDebug(views) << "Enter TermWidget::getTabTitleFormat";
    return m_tabFormat.currentTabFormat;
}

QString TermWidget::getRemoteTabTitleFormat()
{
    // qCDebug(views) << "Enter TermWidget::getRemoteTabTitleFormat";
    return m_tabFormat.remoteTabFormat;
}

QString TermWidget::getCurrentTabTitleFormat()
{
    qCDebug(views) << "Enter TermWidget::getCurrentTabTitleFormat";
    // 连接远程
    if (isConnectRemote()) {
        qCDebug(views) << "Branch: isConnectRemote is true, returning remoteTabFormat";
        return m_tabFormat.remoteTabFormat;
    }

    // 未连接远程
    qCDebug(views) << "Branch: isConnectRemote is false, returning currentTabFormat";
    return m_tabFormat.currentTabFormat;
}

QString TermWidget::encode() const
{
    // qCDebug(views) << "Enter TermWidget::encode";
    return m_encode;
}

void TermWidget::setEncode(const QString &encode)
{
    // qCDebug(views) << "Enter TermWidget::setEncode:" << encode;
    m_encode = encode;
}

bool TermWidget::isConnectRemote() const
{
    // qCDebug(views) << "Enter TermWidget::isConnectRemote";
    return m_isConnectRemote;
}

void TermWidget::setIsConnectRemote(bool isConnectRemote)
{
    // qCDebug(views) << "Enter TermWidget::setIsConnectRemote:" << isConnectRemote;
    m_isConnectRemote = isConnectRemote;
}

void TermWidget::modifyRemoteTabTitle(ServerConfig remoteConfig)
{
    qCDebug(views) << "Enter TermWidget::modifyRemoteTabTitle";
    // 远程主机名
    m_remoteTabArgs[REMOTE_HOST_NAME] = remoteConfig.m_address;
    // 用户名 %u
    m_remoteTabArgs[USER_NAME] = remoteConfig.m_userName;
    // 用户名@ %U
    m_remoteTabArgs[USER_NAME_L] = remoteConfig.m_userName + QString("@");

    // 标签标题变化
    emit termTitleChanged(getTabTitle());
}

bool TermWidget::enterSzCommand() const
{
    // qCDebug(views) << "Enter TermWidget::enterSzCommand";
    return m_enterSzCommand;
}

void TermWidget::setEnterSzCommand(bool enterSzCommand)
{
    // qCDebug(views) << "Enter TermWidget::setEnterSzCommand:" << enterSzCommand;
    m_enterSzCommand = enterSzCommand;
}

inline void TermWidget::customContextMenuCall(const QPoint &pos)
{
    qCDebug(views) << "Enter TermWidget::customContextMenuCall";
    /***add by ut001121 zhangmeng 20200514 右键获取焦点, 修复BUG#26003***/
    setFocus();

    // 右键清理插件
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_NONE);

    /*** 修复 bug 28162 鼠标左右键一起按终端会退出 ***/
    if (nullptr == m_menu) {
        qCDebug(views) << "Branch: m_menu is nullptr, creating new DMenu";
        m_menu = new DMenu(this);
        m_menu->setObjectName("TermMenu");//Add by ut001000 renfeixiang 2020-08-13
    }

    m_menu->clear();
    addMenuActions(pos);

    // display the menu.
    m_menu->exec(mapToGlobal(pos));
}

bool TermWidget::isInRemoteServer()
{
    qCDebug(views) << "Enter TermWidget::isInRemoteServer";
    int pid = getForegroundProcessId();
    if (pid <= 0) {
        qCDebug(views) << "Branch: pid is less than or equal to 0, returning false";
        return false;
    }

    QString pidFilepath = "/proc/" + QString::number(pid) + "/comm";
    QFile pidFile(pidFilepath);
    if (pidFile.exists()) {
        qCDebug(views) << "Branch: pidFile exists, opening file";
        pidFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QString commString(pidFile.readLine());
        pidFile.close();
        if ("expect" == commString.trimmed()) {
            qCDebug(views) << "Branch: commString is expect, returning true";
            return true;
        }
    }
    qCDebug(views) << "Branch: commString is not expect, returning false";
    return false;
}

void TermWidget::setTermOpacity(qreal opacity)
{
    qCDebug(views) << "Enter TermWidget::setTermOpacity";
    //这里再次判断一遍，因为刚启动时，还是需要判断一次当前是否开启了窗口特效
    qreal termOpacity = opacity;
    if (!Service::instance()->isWindowEffectEnabled())
        termOpacity = 1.0;

    setTerminalOpacity(termOpacity);
    /******* Modify by n014361 wangpeili 2020-01-04: 修正实时设置透明度问题************/
    hide();
    show();
    /********************* Modify by n014361 wangpeili End ************************/
}

void TermWidget::setTermFont(const QString &fontName)
{
    qCDebug(views) << "Enter TermWidget::setTermFont:" << fontName;
    QFont font = getTerminalFont();
    font.setFamily(fontName);
    setTerminalFont(font);
}

void TermWidget::setTermFontSize(const int fontSize)
{
    qCDebug(views) << "Enter TermWidget::setTermFontSize:" << fontSize;
    QFont font = getTerminalFont();
    font.setFixedPitch(true);
    font.setPointSize(fontSize);
    setTerminalFont(font);
}

void TermWidget::setCursorShape(int shape)
{
    qCDebug(views) << "Enter TermWidget::setCursorShape:" << shape;
    Konsole::Emulation::KeyboardCursorShape cursorShape = Konsole::Emulation::KeyboardCursorShape(shape);
    setKeyboardCursorShape(cursorShape);
}

void TermWidget::setPressingScroll(bool enable)
{
    qCDebug(views) << "Enter TermWidget::setPressingScroll:" << enable;
    if (enable)
        setMotionAfterPasting(2);
    else
        setMotionAfterPasting(0);
}

void TermWidget::selectEncode(QString encode)
{
    qCDebug(views) << "Enter TermWidget::selectEncode:" << encode;
    // 直接设置终端
    setTextCodec(QTextCodec::codecForName(encode.toUtf8()));
    // 是否连接远程
    if (!isConnectRemote()) {
        // 记录当前的encode
        setEncode(encode);
        qCInfo(views) << "current encode " << encode;
    } else {
        // 记录远程的encode
        setRemoteEncode(encode);
        qCInfo(views) << "Remote encode " << encode;
    }
}

void TermWidget::onSettingValueChanged(const QString &keyName)
{
    qCInfo(views) << "Setting Value Changed! Current Config's key:" << keyName;
    if ("basic.interface.opacity" == keyName) {
        qCDebug(views) << "Enter TermWidget::onSettingValueChanged: basic.interface.opacity";
        setTermOpacity(Settings::instance()->opacity());
        return;
    }

    if ("basic.interface.font" == keyName) {
        qCDebug(views) << "Enter TermWidget::onSettingValueChanged: basic.interface.font";
        setTermFont(Settings::instance()->fontName());
        /******** Add by nt001000 renfeixiang 2020-05-20:增加字体变化时设置雷神窗口最小高度 Begin***************/
        m_page->parentMainWindow()->setWindowMinHeightForFont();
        /******** Add by ut001000 renfeixiang 2020-08-07:字体改变时改变大小，bug#41436***************/
        m_page->parentMainWindow()->updateMinHeight();
        /******** Add by nt001000 renfeixiang 2020-05-20:增加字体变化时设置雷神窗口最小高度 End***************/
        return;
    }

    if ("basic.interface.font_size" == keyName) {
        qCDebug(views) << "Enter TermWidget::onSettingValueChanged: basic.interface.font_size";
        setTermFontSize(Settings::instance()->fontSize());
        /******** Add by nt001000 renfeixiang 2020-05-20:增加字体大小变化时设置雷神窗口最小高度 Begin***************/
        m_page->parentMainWindow()->setWindowMinHeightForFont();
        /******** Add by ut001000 renfeixiang 2020-08-07:字体大小改变时改变大小，bug#41436***************/
        m_page->parentMainWindow()->updateMinHeight();
        /******** Add by nt001000 renfeixiang 2020-05-20:增加字体大小变化时设置雷神窗口最小高度 End***************/
        return;
    }

    if ("advanced.cursor.cursor_shape" == keyName) {
        qCDebug(views) << "Enter TermWidget::onSettingValueChanged: advanced.cursor.cursor_shape";
        setCursorShape(Settings::instance()->cursorShape());
        return;
    }

    if ("advanced.cursor.cursor_blink" == keyName) {
        qCDebug(views) << "Enter TermWidget::onSettingValueChanged: advanced.cursor.cursor_blink";
        setBlinkingCursor(Settings::instance()->cursorBlink());
        return;
    }

    if ("advanced.cursor.set_cursor_position" == keyName) {
        enableSetCursorPosition(Settings::instance()->enableSetCursorPosition());
    }

    if ("advanced.scroll.scroll_on_key" == keyName) {
        qCDebug(views) << "Enter TermWidget::onSettingValueChanged: advanced.scroll.scroll_on_key";
        setPressingScroll(Settings::instance()->PressingScroll());
        return;
    }

    if ("basic.interface.theme" == keyName) {
        qCDebug(views) << "Enter TermWidget::onSettingValueChanged: basic.interface.theme";
        return;
    }
    // 这里只是立即生效一次，真正生效起作用的地方在初始的connect中
    if ("advanced.cursor.auto_copy_selection" == keyName) {
        qCDebug(views) << "Enter TermWidget::onSettingValueChanged: advanced.cursor.auto_copy_selection";
        if (Settings::instance()->IsPasteSelection())
            copyClipboard();
        else
            QApplication::clipboard()->clear(QClipboard::Clipboard);

        return;
    }

    if ("advanced.scroll.scroll_on_output" == keyName) {
        qCInfo(views) << "settingValue[" << keyName << "] changed to " << Settings::instance()->OutputtingScroll()
                << ", auto effective when happen";
        return;
    }
    
    if ("advanced.cursor.include_special_characters_in_double_click_selections" == keyName) {
        setTerminalWordCharacters(Settings::instance()->wordCharacters());
        return;
    }

    if ("advanced.shell.disable_ctrl_flow" == keyName) {
        setFlowControlEnabled(!Settings::instance()->disableControlFlow());
        return;
    }

    if ("advanced.debuginfod.enable_debuginfod" == keyName) {
        if (!hasRunningProcess()) {
            if (Settings::instance()->enableDebuginfod()) {
                sendText(QString("test -z $DEBUGINFOD_URLS && export DEBUGINFOD_URLS=\"%1\"\n").arg(Settings::instance()->debuginfodUrls()));
            } else {
                sendText("test -z $DEBUGINFOD_URLS || unset DEBUGINFOD_URLS\n");
            }
        } else {
            // Todo(ArchieMeng): Should handle the situation when there is a running process. It should wait until all running processes being exited.
            showShellMessage(tr("The debuginfod settings will be effective after restart"));
        }
        return;
    }

    qInfo() << "settingValue[" << keyName << "] changed is not effective";
}

void TermWidget::onDropInUrls(const char *urls)
{
    qCDebug(views) << "Enter TermWidget::onDropInUrls";
    QString strUrls = QString::fromLocal8Bit(urls);
    qCInfo(views) << "recv urls:" << strUrls;
    if (isConnectRemote()) {
        qCDebug(views) << "Branch: isConnectRemote is true, uploading file";
        // 远程管理连接中
        QString strTxt = "sz ";
        strTxt += strUrls;
        // 上传文件
        parentPage()->parentMainWindow()->remoteUploadFile(strTxt);
    } else {
        qCDebug(views) << "Branch: isConnectRemote is false, sending text";
        // 发送字符
        sendText(strUrls);
    }
}


inline void TermWidget::onTouchPadSignal(QString name, QString direction, int fingers)
{
    qCDebug(views) << "Enter TermWidget::onTouchPadSignal";
    // 当前窗口被激活,且有焦点
    if (isActiveWindow() && hasFocus() && Settings::instance()->ScrollWheelZoom()) {
        if (name == "pinch" && fingers == 2) {
            if (direction == "in") {
                // 捏合 in是手指捏合的方向 向内缩小
                zoomOut();  // zoom out 缩小
            } else if (direction == "out") {
                // 捏合 out是手指捏合的方向 向外放大
                zoomIn();   // zoom in 放大
            }
        }
    }
}

void TermWidget::onShellMessage(QString currentShell, bool isSuccess)
{
    qCDebug(views) << "Enter TermWidget::onShellMessage";
    if (isSuccess) {
        qCDebug(views) << "Branch: isSuccess is true";
        // 替换了shell提示
        QString strSetShell = Settings::instance()->shellPath();
        QString strShellWarning = QObject::tr("Could not find \"%1\", starting \"%2\" instead. Please check your shell profile.").arg(strSetShell).arg(currentShell);
        showShellMessage(strShellWarning);
    } else {
        qCDebug(views) << "Branch: isSuccess is false";
        // 启动shell失败
        QString strShellNoFound;
        if(QFile::exists(currentShell)) {
            qCDebug(views) << "Branch: currentShell exists";
            strShellNoFound = QObject::tr("Could not open \"%1\", unable to run it").arg(currentShell);
        } else {
            qCDebug(views) << "Branch: currentShell does not exist";
            strShellNoFound = QObject::tr("Could not find \"%1\", unable to run it").arg(currentShell);
        }

        showShellMessage(strShellNoFound);
    }
}

void TermWidget::wheelEvent(QWheelEvent *event)
{
    int directionY = event->angleDelta().y();
    if (directionY == 0) {
        // 兼容触控板等设备
        directionY = event->pixelDelta().y();
    }
    bool handled = false;
    // 当前窗口被激活,且有焦点
    if (isActiveWindow() && hasFocus()) {
        // Ctrl + 滚轮 缩放字体（保留原行为，由开关控制）
        if (Settings::instance()->ScrollWheelZoom() && event->modifiers() == Qt::ControlModifier) {
            if (directionY < 0) {
                qCDebug(views) << "Branch: directionY is less than 0, zooming out";
                zoomOut();  // 缩小
            } else {
                qCDebug(views) << "Branch: directionY is greater than 0, zooming in";
                zoomIn();   // 放大
            }
            handled = true;
        }
    }

    if (!handled) {
        QTermWidget::wheelEvent(event);
    }
}

void TermWidget::showFlowMessage(bool show)
{
    qCDebug(views) << "Enter TermWidget::showFlowMessage";
    if (nullptr == m_flowMessage) {
        qCDebug(views) << "Branch: m_flowMessage is nullptr, creating new DFloatingMessage";
        m_flowMessage = new DFloatingMessage(DFloatingMessage::ResidentType, this);
        m_flowMessage->setIcon(QIcon(":icons/deepin/builtin/warning.svg"));
        QString strText = QObject::tr("Output has been suspended by pressing Ctrl+S. Pressing Ctrl+Q to resume.");
        m_flowMessage->setMessage(strText);
        DMessageManager::instance()->sendMessage(this, m_flowMessage);
    }
    m_flowMessage->setVisible(show);
}

void TermWidget::showShellMessage(QString strWarnings)
{
    qCDebug(views) << "Enter TermWidget::showShellMessage";
    // 初始化悬浮框
    DFloatingMessage *shellWarningsMessage = new DFloatingMessage(DFloatingMessage::ResidentType, this);
    // 关闭悬浮框时销毁
    connect(shellWarningsMessage, &DFloatingMessage::closeButtonClicked, shellWarningsMessage, &DFloatingMessage::deleteLater);
    // 设置icon和文字
    shellWarningsMessage->setIcon(QIcon(":icons/deepin/builtin/warning.svg"));
    shellWarningsMessage->setMessage(strWarnings);
    // 调用DTK的方法关闭悬浮框
    DMessageManager::instance()->sendMessage(this, shellWarningsMessage);
}


