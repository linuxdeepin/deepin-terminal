// SPDX-FileCopyrightText: 2019 ~ 2023 UnionTech Software Technology Co., Ltd.
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
#include <DApplicationHelper>
#include <DPaletteHelper>
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
TermWidget::TermWidget(const TermProperties &properties, QWidget *parent) : QTermWidget(0, parent), m_properties(properties)
{
    Utils::set_Object_Name(this);
    // 窗口数量加1
    WindowsManager::instance()->terminalCountIncrease();
    initTabTitle();
    //qInfo() << " TermWidgetparent " << parentWidget();
    m_page = static_cast<TermWidgetPage *>(parentWidget());
    setContextMenuPolicy(Qt::CustomContextMenu);

    qInfo() << "Setting initial history size:" << Settings::instance()->historySize();
    setHistorySize(Settings::instance()->historySize());
    setTerminalWordCharacters(Settings::instance()->wordCharacters());

    // 设置debuginfod
    if (Settings::instance()->enableDebuginfod()) {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        if (!env.contains("DEBUGINFOD_URLS")) {
            env.insert("DEBUGINFOD_URLS", Settings::instance()->debuginfodUrls());
            setEnvironment(env.toStringList());
        }
    }

    QString strShellPath = Settings::instance()->shellPath();
    // set shell program
    qInfo() << "set shell program : " << strShellPath;
    setShellProgram(strShellPath);
    /******** Modify by ut000610 daizhengwen 2020-07-08:初始化透明度 Begin***************/
    // 若没有窗口特效，则不生效
    // 若有窗口特效，生效

    // 此方法会丢失焦点（show hide方法丢失焦点）
    // setTermOpacity(Settings::instance()->opacity());

    // 底层方法，设置当前窗口的透明度
    if (DWindowManagerHelper::instance()->hasComposite()) {
        // 判断当前是否有窗口特效
        setTerminalOpacity(Settings::instance()->opacity());
    }
    /********************* Modify by ut000610 daizhengwen End ************************/
    QString colorTheme = "Dark";
    // 没有主题配置
    if (!Settings::instance()->colorScheme().isEmpty()) {
        colorTheme = Settings::instance()->colorScheme();
    }
    setTheme(colorTheme);

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
        qInfo() << "Execute args:" << argList;
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
    // 是否允许移动光标
    enableSetCursorPosition(Settings::instance()->enableSetCursorPosition());

    // 按键滚动
    setPressingScroll(Settings::instance()->PressingScroll());

    setMinimumSize(MIN_WIDTH, MIN_HEIGHT);

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
        qInfo() << "run cmd:" << args;
        args.append("\n");
        if (!m_properties[KeepOpen].toBool())
            args.append("exit\n");

        sendText(args);
    }

    setFocusPolicy(Qt::NoFocus);

    setFlowControlEnabled(!Settings::instance()->disableControlFlow());

    TermWidgetPage *parentPage = qobject_cast<TermWidgetPage *>(parent);
    //qInfo() << parentPage;
    connect(this, &QTermWidget::uninstallTerminal, parentPage, &TermWidgetPage::uninstallTerminal);
}

void TermWidget::initConnections()
{
    // 输出滚动，会在每个输出判断是否设置了滚动，即时设置
    connect(this, &QTermWidget::receivedData, this, &TermWidget::onQTermWidgetReceivedData);

    // 接收到输出
    connect(this, &TermWidget::receivedData, this, &TermWidget::onTermWidgetReceivedData);

    // 接收到拖拽文件的Urls数据
    connect(this, &QTermWidget::sendUrlsToTerm, this, &TermWidget::onDropInUrls);

    connect(this, &QTermWidget::urlActivated, this, &TermWidget::onUrlActivated);

    connect(this, &QWidget::customContextMenuRequested, this, &TermWidget::customContextMenuCall);

    // 主题变化只能从公共方法发出信号通知全局
    connect(Service::instance(), &Service::changeColorTheme, this, &TermWidget::onColorThemeChanged);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &TermWidget::onThemeChanged);

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
    connect(Settings::instance(), &Settings::historySizeChanged, this, [this](int newHistorySize) {
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
    QFont font = getTerminalFont();
    font.setFamily(Settings::instance()->fontName());
    font.setPointSize(Settings::instance()->fontSize());
    setTerminalFont(font);
}

inline void TermWidget::onSig_noMatchFound()
{
    parentPage()->setMismatchAlert(true);
}

inline void TermWidget::onQTermWidgetReceivedData(QString value)
{
    Q_UNUSED(value)
    // 完善终端输出滚动相关功能，默认设置为"智能滚动"(即滚动条滑到最底下时自动滚动)
    if (!Settings::instance()->OutputtingScroll()) {
        setIsAllowScroll(true);
        return;
    }

    // 获取是否允许输出时滚动
    if (getIsAllowScroll()) {
        // 允许,则滚动到最新位置
        setTrackOutput(Settings::instance()->OutputtingScroll());
    } else {
        // 不允许,则不滚动
        // 将标志位置位
        setIsAllowScroll(true);
    }
}

inline void TermWidget::onTermWidgetReceivedData(QString value)
{
    //前提：
    //启动终端ForegroundPid:A
    //远程开始、输入信息中、远程中：ForegroundPid:B
    //远程结束ForegroundPid:A
    //远程开始时，快速ctrl+c，也会有ForegroundPid：A-》B-》A的过程

    //准备输入密码，且 ForegroundPid 不等于A时，为有效准备
    if (m_remotePasswordIsReady && getForegroundProcessId() != m_remoteMainPid) {
        //匹配关键字
        if (value.toLower().contains("password:")
                || value.toLower().contains("enter passphrase for key")) {
            //输入密码,密码不为空，则发送
            if (!m_remotePassword.isEmpty())
                sendText(m_remotePassword + "\r");
            emit remotePasswordHasInputed();
        }
        //第一次远程时，需要授权
        if (value.toLower().contains("yes/no")) {
            sendText("yes\r");
        }

    }
    //若ForegroundPid等于A，则代表远程结束，如开始连接时立刻ctrl+c
    if (m_remotePasswordIsReady && getForegroundProcessId() == m_remoteMainPid) {
        m_remotePasswordIsReady = false;
    }

    /******** Modify by ut000610 daizhengwen 2020-05-25: quit download****************/
    if (value.contains("Transfer incomplete")) {
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_C, Qt::ControlModifier);
        QApplication::sendEvent(focusWidget(), &keyPress);
    }
    if (value.endsWith("\b \b #")) {                     // 结束的时候有乱码的话，将它清除
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
    // 判断是否此时退出远程
    if (!isInRemoteServer()) {
        qInfo() << "exit remote";
        setIsConnectRemote(false);
        // 还原编码
        setTextCodec(QTextCodec::codecForName(encode().toLocal8Bit()));
        qInfo() << "current encode " << encode();
        setBackspaceMode(m_backspaceMode);
        setDeleteMode(m_deleteMode);
        emit Service::instance()->checkEncode(encode());
    }
}

inline void TermWidget::onUrlActivated(const QUrl &url, bool fromContextMenu)
{
    if (QApplication::keyboardModifiers() & Qt::ControlModifier || fromContextMenu)
        QDesktopServices::openUrl(url);
}

inline void TermWidget::onColorThemeChanged(const QString &colorTheme)
{
    setTheme(colorTheme);
}

inline void TermWidget::onThemeChanged(DGuiApplicationHelper::ColorType themeType)
{
    Q_UNUSED(themeType);
    if ("System Theme" == Settings::instance()->colorScheme()) {
        QString theme;
        if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
            theme = "Dark";
        } else {
            theme = "Light";
        }
        setColorScheme(theme);
    }
}

inline void TermWidget::onTermIsIdle(bool bIdle)
{
    emit termIsIdle(m_page->identifier(), bIdle);
}

inline void TermWidget::onTitleChanged()
{
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
    if (Settings::instance()->IsPasteSelection() && enable) {
        qInfo() << "hasCopySelection";
        QString strSelected = selectedText();
        QApplication::clipboard()->setText(strSelected, QClipboard::Clipboard);
    }
}

inline void TermWidget::onWindowEffectEnabled(bool isWinEffectEnabled)
{
    if (isWinEffectEnabled)
        this->setTermOpacity(Settings::instance()->opacity());
    else
        this->setTermOpacity(1.0);
}

TermWidget::~TermWidget()
{
    // 窗口减1
    WindowsManager::instance()->terminalCountReduce();
}

TermWidgetPage *TermWidget::parentPage()
{
    return  m_page;
}

void TermWidget::onTitleArgsChange(QString key, QString value)
{
    // tab获取参数
    m_tabArgs[key] = value;
    // 获取当前目录短 由长到短
    if (DIR_L == key) {
        QString dir = value;
        // 当前目录短（若有优化方法请及时告知）
        if ("~" == dir || dir.startsWith(QDir::homePath())) {
            // 出现家目录~的情况
            QString homePath = QDir::homePath();
            QStringList pathList;
            if ("~" == dir)
                pathList = homePath.split("/");
            else
                pathList = dir.split("/");
            // 当前目录短，返回当前文件夹名
            m_tabArgs[DIR_S] = pathList.value(pathList.count() - 1);
            // 当前目录长对于~的处理 => 传过来的不是~但要填进去~和提示符保持一致
            dir.replace(homePath, "~");
            m_tabArgs[DIR_L] = dir;
        } else if ("/" == dir) {
            // 出现根目录/的情况
            m_tabArgs[DIR_S] = "/";
        } else {
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
    // 主机名变化
    QString hostName = QHostInfo::localHostName();
    m_tabArgs[LOCAL_HOST_NAME] = hostName;
    // 发送标签标题变化
    emit termTitleChanged(getTabTitle());
}

inline void TermWidget::onCopy()
{
    copyClipboard();
}

inline void TermWidget::onPaste()
{
    pasteClipboard();
}

inline void TermWidget::onOpenFileInFileManager()
{
    //DDesktopServices::showFolder(QUrl::fromLocalFile(workingDirectory()));

    QProcess process;
    //未选择内容
    if (selectedText().isEmpty()) {
        DDesktopServices::showFolder(QUrl::fromLocalFile(workingDirectory()));
        return;
    }

    QFileInfo fi(workingDirectory() + "/" + selectedText());
    //选择的内容是文件或者文件夹
    if (fi.isDir()) {
        DDesktopServices::showFolder(QUrl::fromLocalFile(fi.filePath()));
        return;
    } else if (fi.isFile()) {
        DDesktopServices::showFileItem(QUrl::fromLocalFile(fi.filePath()));
        return;
    }

    //选择的文本不是文件也不是文件夹
    DDesktopServices::showFolder(QUrl::fromLocalFile(workingDirectory()));
}

/*** 修复 bug 28162 鼠标左右键一起按终端会退出 ***/
void TermWidget::addMenuActions(const QPoint &pos)
{
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

    QAction *action = 0;
    action = m_menu->addAction(tr("Horizontal split"), this, &TermWidget::onHorizontalSplit);
    action->setEnabled(canSplit(Qt::Vertical));
    action = m_menu->addAction(tr("Vertical split"), this, &TermWidget::onVerticalSplit);
    action->setEnabled(canSplit(Qt::Horizontal));

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
    // menu关闭与分屏同时进行时，会导致QT计算光标位置异常。
    QTimer::singleShot(10, this, &TermWidget::splitHorizontal);
}

inline void TermWidget::onVerticalSplit()
{
    // menu关闭与分屏同时进行时，会导致QT计算光标位置异常。
    QTimer::singleShot(10, this, &TermWidget::splitVertical);
}

inline void TermWidget::splitHorizontal()
{
    parentPage()->split(Qt::Horizontal);
}

inline void TermWidget::splitVertical()
{
    parentPage()->split(Qt::Vertical);
}

inline void TermWidget::onCloseCurrWorkSpace()
{
    parentPage()->closeSplit(parentPage()->currentTerminal());
}

inline void TermWidget::onCloseOtherWorkSpaces()
{
    parentPage()->closeOtherTerminal();
}

inline void TermWidget::onCreateNewTab()
{
    parentPage()->parentMainWindow()->createNewTab();
}

inline void TermWidget::onSwitchFullScreen()
{
    parentPage()->parentMainWindow()->switchFullscreen();
}

inline void TermWidget::openBing()
{
    QString strUrl = "https://cn.bing.com/search?q=" + selectedText();
    openUrl(strUrl);
}

inline void TermWidget::openBaidu()
{
    QString strUrl = "https://www.baidu.com/s?wd=" + selectedText();
    openUrl(strUrl);
}

inline void TermWidget::openGithub()
{
    QString strUrl = "https://github.com/search?q=" + selectedText();
    openUrl(strUrl);
}

inline void TermWidget::openStackOverflow()
{
    QString strUrl = "https://stackoverflow.com/search?q=" + selectedText();
    openUrl(strUrl);
}

inline void TermWidget::onShowSearchBar()
{
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_SEARCHBAR);
}

inline void TermWidget::onShowEncoding()
{
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);
}

inline void TermWidget::onShowCustomCommands()
{
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);
}

inline void TermWidget::onShowRemoteManagement()
{
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);
}

inline void TermWidget::onUploadFile()
{
    parentPage()->parentMainWindow()->remoteUploadFile();
}

inline void TermWidget::onDownloadFile()
{
    parentPage()->parentMainWindow()->remoteDownloadFile();
}

inline void TermWidget::onShowSettings()
{
    Service::instance()->showSettingDialog(parentPage()->parentMainWindow());
}

inline void TermWidget::openUrl(QString strUrl)
{
    QDesktopServices::openUrl(QUrl(strUrl));
}

inline QString TermWidget::getFormatFileName(QString selectedText)
{
    QString fileName = selectedText.trimmed();
    if ((fileName.startsWith("'") && fileName.endsWith("'"))
            || (fileName.startsWith("\"") && fileName.endsWith("\""))) {
        fileName = fileName.remove(0, 1);
        fileName = fileName.remove(fileName.length() - 1, 1);
        qInfo() << "fileName is :" << fileName;
    }

    return fileName;
}

inline QString TermWidget::getFilePath(QString fileName)
{
    //如果fileName本身已经是一个文件路径
    if (fileName.startsWith("/"))
        return fileName;

    return workingDirectory() + "/" + fileName;
}

void TermWidget::inputRemotePassword(const QString &remotePassword)
{
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

void TermWidget::changeTitleColor(int lightness)
{
    if (lightness >= 192) {
        DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::LightType);
    } else {
        DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::DarkType);
    }
}

void TermWidget::setTheme(const QString &colorTheme)
{
    QString theme = colorTheme;
    // 跟随系统
    if ("System Theme" == colorTheme) {
        DGuiApplicationHelper::instance()->setPaletteType(DGuiApplicationHelper::UnknownType);
        if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
            theme = "Dark";
        } else {
            theme = "Light";
        }
        setColorScheme(theme);
        return;
    }

    // 自定义主题
    if (colorTheme == Settings::instance()->m_configCustomThemePath) {
        DGuiApplicationHelper::ColorType systemTheme = DGuiApplicationHelper::DarkType;
        if ("Light" == Settings::instance()->themeSetting->value("CustomTheme/TitleStyle")) {
            systemTheme = DGuiApplicationHelper::LightType;
        }
        DGuiApplicationHelper::instance()->setPaletteType(systemTheme);
        setColorScheme(theme);
        return;
    }

    // 设置主题
    int lightness = setColorScheme(theme);
    // 设置系统主题
    changeTitleColor(lightness);
}

inline void TermWidget::onOpenFile()
{
    QString fileName = getFormatFileName(selectedText());
    QString filePath = getFilePath(fileName);
    QUrl fileUrl = QUrl::fromLocalFile(filePath);
    QDesktopServices::openUrl(fileUrl);
}

QString TermWidget::RemoteEncode() const
{
    return m_remoteEncode;
}

void TermWidget::setRemoteEncode(const QString &RemoteEncode)
{
    m_remoteEncode = RemoteEncode;
}

void TermWidget::setBackspaceMode(const EraseMode &backspaceMode)
{
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
        qInfo() << "tty erase : " << QByteArray(&ch, length).toHex();
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
        qInfo() << "tty erase : " << QByteArray(&ch, length).toHex();
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

void TermWidget::setTabFormat(const QString &tabFormat)
{
    // 非全局设置优先级更高
    if (m_tabFormat.isGlobal != false && m_tabFormat.currentTabFormat != tabFormat) {
        m_tabFormat.currentTabFormat = tabFormat;
        // 有变化，就将变化发出，但此时可能显示的不是变化的项
        emit termTitleChanged(getTabTitle());
    }
}

void TermWidget::setRemoteTabFormat(const QString &remoteTabFormat)
{
    // 非全局设置优先级更高
    if (m_tabFormat.isGlobal != false && m_tabFormat.remoteTabFormat != remoteTabFormat) {
        m_tabFormat.remoteTabFormat = remoteTabFormat;
        // 有变化，就将变化发出，但此时可能显示的不是变化的项
        emit termTitleChanged(getTabTitle());
    }
}

void TermWidget::renameTabFormat(const QString &tabFormat, const QString &remoteTabFormat)
{
    // 重命名优先级高
    m_tabFormat.currentTabFormat = tabFormat;
    m_tabFormat.remoteTabFormat = remoteTabFormat;
    m_tabFormat.isGlobal = false;
    emit termTitleChanged(getTabTitle());
}

QString TermWidget::getTabTitle()
{
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
}

void TermWidget::initTabTitleArgs()
{
    QStringList strTabArgs = TAB_ARGS.split(" ");
    // 填充标签标题参数
    for (QString arg : strTabArgs)
        m_tabArgs.insert(arg, " ");

    // 填充远程标题标签参数
    QStringList strRemoteTabArgs = REMOTE_ARGS.split(" ");
    for (QString arg : strRemoteTabArgs)
        m_remoteTabArgs.insert(arg, " ");

    qInfo() << "Tab args init! tab title count : " << m_tabArgs.count() << " remote title count : " << m_remoteTabArgs.count();
}

QString TermWidget::getTabTitleFormat()
{
    return m_tabFormat.currentTabFormat;
}

QString TermWidget::getRemoteTabTitleFormat()
{
    return m_tabFormat.remoteTabFormat;
}

QString TermWidget::getCurrentTabTitleFormat()
{
    // 连接远程
    if (isConnectRemote())
        return m_tabFormat.remoteTabFormat;

    // 未连接远程
    return m_tabFormat.currentTabFormat;
}

QString TermWidget::encode() const
{
    return m_encode;
}

void TermWidget::setEncode(const QString &encode)
{
    m_encode = encode;
}

bool TermWidget::isConnectRemote() const
{
    return m_isConnectRemote;
}

void TermWidget::setIsConnectRemote(bool isConnectRemote)
{
    m_isConnectRemote = isConnectRemote;
}

void TermWidget::modifyRemoteTabTitle(ServerConfig remoteConfig)
{
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
    return m_enterSzCommand;
}

void TermWidget::setEnterSzCommand(bool enterSzCommand)
{
    m_enterSzCommand = enterSzCommand;
}

inline void TermWidget::customContextMenuCall(const QPoint &pos)
{
    /***add by ut001121 zhangmeng 20200514 右键获取焦点, 修复BUG#26003***/
    setFocus();

    // 右键清理插件
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_NONE);

    /*** 修复 bug 28162 鼠标左右键一起按终端会退出 ***/
    if (nullptr == m_menu) {
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
    int pid = getForegroundProcessId();
    if (pid <= 0)
        return false;

    QString pidFilepath = "/proc/" + QString::number(pid) + "/comm";
    QFile pidFile(pidFilepath);
    if (pidFile.exists()) {
        pidFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QString commString(pidFile.readLine());
        pidFile.close();
        if ("expect" == commString.trimmed())
            return true;
    }
    return false;
}

bool TermWidget::canSplit(Qt::Orientation ori) {
    qDebug() << "CanSplit:" << ori;
    QSplitter *splitter = qobject_cast<QSplitter *>(this->parentWidget());
    int minimumSize = ori == Qt::Horizontal ? TermWidget::MIN_WIDTH : TermWidget::MIN_HEIGHT;
    if (splitter) {
        if (splitter->orientation() == ori) {
            QList<int> sizes = splitter->sizes();
            // new term has same size portion as the current one.
            sizes.append(sizes.at(splitter->indexOf(this)));

            double sum = 0;
            for (int i = 0; i < sizes.count(); i++) {
                sum += sizes.at(i);
            }

            for(int i = 0; i < sizes.count(); i++) {
                int totalSize = ori == Qt::Horizontal ? splitter->width() : splitter->height();
                int actualSize = (totalSize) * (sizes.at(i) / sum);
                if (actualSize < minimumSize)
                    return false;
            }
        } else {
            int splitterSize = ori == Qt::Horizontal ? splitter->width() : splitter->height();
            if (splitterSize / 2.0 < minimumSize)
                return false;
        }
    }

    return true;
}


void TermWidget::setTermOpacity(qreal opacity)
{
    //这里再次判断一遍，因为刚启动时，还是需要判断一次当前是否开启了窗口特效
    qreal termOpacity = opacity;
    if (!DWindowManagerHelper::instance()->hasComposite())
        termOpacity = 1.0;

    setTerminalOpacity(termOpacity);
    /******* Modify by n014361 wangpeili 2020-01-04: 修正实时设置透明度问题************/
    hide();
    show();
    /********************* Modify by n014361 wangpeili End ************************/
}

void TermWidget::setTermFont(const QString &fontName)
{
    QFont font = getTerminalFont();
    font.setFamily(fontName);
    setTerminalFont(font);
}

void TermWidget::setTermFontSize(const int fontSize)
{
    QFont font = getTerminalFont();
    font.setFixedPitch(true);
    font.setPointSize(fontSize);
    setTerminalFont(font);
}

void TermWidget::setCursorShape(int shape)
{
    Konsole::Emulation::KeyboardCursorShape cursorShape = Konsole::Emulation::KeyboardCursorShape(shape);
    setKeyboardCursorShape(cursorShape);
}

void TermWidget::setPressingScroll(bool enable)
{
    if (enable)
        setMotionAfterPasting(2);
    else
        setMotionAfterPasting(0);
}

void TermWidget::selectEncode(QString encode)
{
    // 直接设置终端
    setTextCodec(QTextCodec::codecForName(encode.toUtf8()));
    // 是否连接远程
    if (!isConnectRemote()) {
        // 记录当前的encode
        setEncode(encode);
        qInfo() << "current encode " << encode;
    } else {
        // 记录远程的encode
        setRemoteEncode(encode);
        qInfo() << "Remote encode " << encode;
    }
}

void TermWidget::onSettingValueChanged(const QString &keyName)
{
    qInfo() << "onSettingValueChanged:" << keyName;
    if ("basic.interface.opacity" == keyName) {
        setTermOpacity(Settings::instance()->opacity());
        return;
    }

    if ("basic.interface.font" == keyName) {
        setTermFont(Settings::instance()->fontName());
        /******** Add by nt001000 renfeixiang 2020-05-20:增加字体变化时设置雷神窗口最小高度 Begin***************/
        m_page->parentMainWindow()->setWindowMinHeightForFont();
        /******** Add by ut001000 renfeixiang 2020-08-07:字体改变时改变大小，bug#41436***************/
        m_page->parentMainWindow()->updateMinHeight();
        /******** Add by nt001000 renfeixiang 2020-05-20:增加字体变化时设置雷神窗口最小高度 End***************/
        return;
    }

    if ("basic.interface.font_size" == keyName) {
        setTermFontSize(Settings::instance()->fontSize());
        /******** Add by nt001000 renfeixiang 2020-05-20:增加字体大小变化时设置雷神窗口最小高度 Begin***************/
        m_page->parentMainWindow()->setWindowMinHeightForFont();
        /******** Add by ut001000 renfeixiang 2020-08-07:字体大小改变时改变大小，bug#41436***************/
        m_page->parentMainWindow()->updateMinHeight();
        /******** Add by nt001000 renfeixiang 2020-05-20:增加字体大小变化时设置雷神窗口最小高度 End***************/
        return;
    }

    if ("advanced.cursor.cursor_shape" == keyName) {
        setCursorShape(Settings::instance()->cursorShape());
        return;
    }

    if ("advanced.cursor.cursor_blink" == keyName) {
        setBlinkingCursor(Settings::instance()->cursorBlink());
        return;
    }

    if ("advanced.cursor.set_cursor_position" == keyName) {
        enableSetCursorPosition(Settings::instance()->enableSetCursorPosition());
    }

    if ("advanced.scroll.scroll_on_key" == keyName) {
        setPressingScroll(Settings::instance()->PressingScroll());
        return;
    }

    if ("basic.interface.theme" == keyName) {
        setTheme(Settings::instance()->colorScheme());
        return;
    }
    // 这里只是立即生效一次，真正生效起作用的地方在初始的connect中
    if ("advanced.cursor.auto_copy_selection" == keyName) {
        if (Settings::instance()->IsPasteSelection())
            copyClipboard();
        else
            QApplication::clipboard()->clear(QClipboard::Clipboard);

        return;
    }

    if ("advanced.scroll.scroll_on_output" == keyName) {
        qInfo() << "settingValue[" << keyName << "] changed to " << Settings::instance()->OutputtingScroll()
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
    QString strUrls = QString::fromLocal8Bit(urls);
    qInfo() << "recv urls:" << strUrls;
    if (isConnectRemote()) {
        // 远程管理连接中
        QString strTxt = "sz ";
        strTxt += strUrls;
        // 上传文件
        parentPage()->parentMainWindow()->remoteUploadFile(strTxt);
    } else {
        // 发送字符
        sendText(strUrls);
    }
}


inline void TermWidget::onTouchPadSignal(QString name, QString direction, int fingers)
{
    qInfo() << name << direction << fingers;
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
    if (isSuccess) {
        // 替换了shell提示
        QString strSetShell = Settings::instance()->shellPath();
        QString strShellWarning = QObject::tr("Could not find \"%1\", starting \"%2\" instead. Please check your shell profile.").arg(strSetShell).arg(currentShell);
        showShellMessage(strShellWarning);
    } else {
        // 启动shell失败
        QString strShellNoFound;
        if (QFile::exists(currentShell))
            strShellNoFound = QObject::tr("Could not open \"%1\", unable to run it").arg(currentShell);
        else
            strShellNoFound = QObject::tr("Could not find \"%1\", unable to run it").arg(currentShell);

        showShellMessage(strShellNoFound);
    }
}

void TermWidget::wheelEvent(QWheelEvent *event)
{
    int directionY = event->angleDelta().y();
    // 当前窗口被激活,且有焦点
    if (isActiveWindow() && hasFocus()) {
        if (Qt::ControlModifier == event->modifiers() && Settings::instance()->ScrollWheelZoom()) {
            if (directionY < 0) {
                // 向下缩小
                zoomOut();  // zoom out 缩小
            } else {
                // 向上放大
                zoomIn();   // zoom in 放大
            }
        } else if ((Qt::ControlModifier | Qt::ShiftModifier) == event->modifiers()) {
            int newOpacity;
            if (directionY < 0) {
                newOpacity = Settings::instance()->settings->option("basic.interface.opacity")->value().toInt() - STEP_OPACITY;
            } else {
                newOpacity = Settings::instance()->settings->option("basic.interface.opacity")->value().toInt() + STEP_OPACITY;
            }
            newOpacity = qBound(0, newOpacity, 100);
            qInfo() << Q_FUNC_INFO << "new opacity:" << newOpacity;
            setTerminalOpacity(newOpacity / 100.f);
            Settings::instance()->settings->option("basic.interface.opacity")->setValue((int)(newOpacity));
        } else {
            QTermWidget::wheelEvent(event);
        }
    } else {
        QTermWidget::wheelEvent(event);
    }
}

void TermWidget::showFlowMessage(bool show)
{
    if (nullptr == m_flowMessage) {
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


