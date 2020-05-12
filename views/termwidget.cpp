#include "termwidget.h"
#include "define.h"
#include "settings.h"
#include "termproperties.h"
#include "mainwindow.h"
#include "shortcutmanager.h"
#include "operationconfirmdlg.h"
#include "utils.h"

#include <DDesktopServices>
#include <DInputDialog>
#include <DApplicationHelper>
#include <DLog>
#include <DDialog>

#include <QApplication>
#include <QDesktopServices>
#include <QMenu>
#include <QVBoxLayout>
#include <QTime>
#include <QApplication>
#include <QClipboard>
#include <QFileInfo>

DWIDGET_USE_NAMESPACE
using namespace Konsole;
TermWidget::TermWidget(TermProperties properties, QWidget *parent) : QTermWidget(0, parent), m_properties(properties)
{
    //qDebug() << " TermWidgetparent " << parentWidget();
    m_Page = static_cast<TermWidgetPage *>(parentWidget());
    setContextMenuPolicy(Qt::CustomContextMenu);

    setHistorySize(5000);

    // set shell program
    QString shell{ getenv("SHELL") };
    setShellProgram(shell.isEmpty() ? "/bin/bash" : shell);
    setTerminalOpacity(Settings::instance()->opacity());
    //setScrollBarPosition(QTermWidget::ScrollBarRight);//commend byq nyq

    /******** Modify by n014361 wangpeili 2020-01-13:              ****************/
    // theme
    QString theme = "Dark";
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        theme = "Light";
    }
    setColorScheme(theme);
    Settings::instance()->setColorScheme(theme);
    // 系统初始打开始终为UTF-8格式。中间的这个参数仅用作通讯使用。
    // 如果需要“记忆”，删除下面这行即可。
    //setTextCodec(QTextCodec::codecForName(Settings::instance()->encoding().toUtf8()));
    setTextCodec(QTextCodec::codecForName("UTF-8"));
    /******** Modify by n014361 wangpeili 2020-03-04: 增加保持打开参数控制，默认自动关闭**/
    setAutoClose(!m_properties.contains(KeepOpen));
    /********************* Modify by n014361 wangpeili End ************************/

    // WorkingDir
    if (m_properties.contains(WorkingDir)) {
        setWorkingDirectory(m_properties[WorkingDir].toString());
    }

    if (m_properties.contains(Execute)) {
        QString args = m_properties[Execute].toString();
        qDebug() << args;
        QStringList argList = args.split(QRegExp(QStringLiteral("\\s+")), QString::SkipEmptyParts);
        if (argList.count() > 0) {
            setShellProgram(argList.at(0));
            argList.removeAt(0);
            if (argList.count()) {
                setArgs(argList);
            }
        }
    }

    // 字体和字体大小, 8#字体立即设置的时候会有BUG显示，做个延迟生效就好了。
    if (Settings::instance()->fontSize() == 8) {
        QTimer::singleShot(10, this, [this]() {
            QFont font = getTerminalFont();
            font.setFamily(Settings::instance()->fontName());
            font.setPointSize(Settings::instance()->fontSize());
            setTerminalFont(font);
        });
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

    // 按键滚动
    if (Settings::instance()->PressingScroll()) {
        qDebug() << "setMotionAfterPasting(2)";
        //setMotionAfterPasting(2);
    } else {
        setMotionAfterPasting(0);
        //qDebug() << "setMotionAfterPasting(0)";
    }

    // 输出滚动，会在每个输出判断是否设置了滚动，即时设置
    connect(this, &QTermWidget::receivedData, this, [this](QString value) {
        Q_UNUSED(value);
        setTrackOutput(Settings::instance()->OutputtingScroll());
    });

#if !(TERMINALWIDGET_VERSION <= QT_VERSION_CHECK(0, 7, 1))
    setBlinkingCursor(Settings::instance()->cursorBlink());
#endif  // !(TERMINALWIDGET_VERSION <= QT_VERSION_CHECK(0, 7, 1))

    connect(this, &QTermWidget::urlActivated, this, [](const QUrl & url, bool fromContextMenu) {
        if (QApplication::keyboardModifiers() & Qt::ControlModifier || fromContextMenu) {
            QDesktopServices::openUrl(url);
        }
    });

    connect(this, &QWidget::customContextMenuRequested, this, &TermWidget::customContextMenuCall);

    connect(DApplicationHelper::instance(),
            &DApplicationHelper::themeTypeChanged,
            this,
    [ = ](DGuiApplicationHelper::ColorType builtInTheme) {
        qDebug() << "themeChanged" << builtInTheme;
        // ThemePanelPlugin *plugin = qobject_cast<ThemePanelPlugin *>(getPluginByName("Theme"));
        QString theme = "Dark";
        if (builtInTheme == DGuiApplicationHelper::LightType) {
            theme = "Light";
        }
        setColorScheme(theme);
        Settings::instance()->setColorScheme(theme);
    });

    connect(this, &QTermWidget::sig_noMatchFound, this, [this]() {
        parentPage()->setMismatchAlert(true);
    });
    /********************* Modify by n014361 wangpeili End ************************/

    connect(this, &QTermWidget::isTermIdle, this, [this](bool bIdle) {
        emit termIsIdle(getSessionId(), bIdle);
    });

    TermWidgetPage *parentPage = qobject_cast<TermWidgetPage *>(parent);
    qDebug() << parentPage << endl;
    connect(this, &QTermWidget::uninstallTerminal, parentPage, &TermWidgetPage::uninstallTerminal);
    // 收到结束信号
    connect(this, &QTermWidget::shellStartedFinished, this, &TermWidget::termInitFinished, Qt::QueuedConnection);
    // 终止下载
    connect(this, &QTermWidget::stopDownload, this, &TermWidget::stopDownload);

    startShellProgram();

    // 增加可以自动运行脚本的命令，不需要的话，可以删除
    if (m_properties.contains(Script)) {
        QString args = m_properties[Script].toString();
        qDebug() << "run cmd:" << args;
        args.append("\n");
        if (!m_properties.contains(KeepOpen)) {
            args.append("exit\n");
        }
        sendText(args);
    }

    connect(this, &QTermWidget::titleChanged, this, [this] { emit termTitleChanged(TermWidget::title()); });
    connect(this, &TermWidget::copyAvailable, this, [this](bool enable) {
        if (Settings::instance()->IsPasteSelection() && enable) {
            qDebug() << "hasCopySelection";
            QString strSelected = selectedText();
            QApplication::clipboard()->setText(strSelected, QClipboard::Clipboard);
        }
    });
    connect(Settings::instance(), &Settings::terminalSettingChanged, this, &TermWidget::onSettingValueChanged);
}

TermWidgetPage *TermWidget::parentPage()
{
    //qDebug() << "parentPage" << parentWidget();
    return  m_Page;
}

void TermWidget::handleTermIdle(bool bIdle)
{
    emit termIsIdle(this->getSessionId(), bIdle);
}

void TermWidget::customContextMenuCall(const QPoint &pos)
{
    // 右键清理插件
    parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_NONE);

    DMenu menu;

    bool isRemoting = isInRemoteServer();

    QList<QAction *> termActions = filterActions(pos);
    for (QAction *&action : termActions) {
        menu.addAction(action);
    }

    if (!menu.isEmpty()) {
        menu.addSeparator();
    }

    // add other actions here.
    if (!selectedText().isEmpty()) {

        menu.addAction(tr("&Copy"), this, [this] { copyClipboard(); });
    }
    if (!QApplication::clipboard()->text(QClipboard::Clipboard).isEmpty()) {
        menu.addAction(tr("&Paste"), this, [this] { pasteClipboard(); });
    }
    /******** Modify by n014361 wangpeili 2020-02-26: 添加打开(文件)菜单功能 **********/
    if (!isRemoting && !selectedText().isEmpty()) {
        QFileInfo tempfile(workingDirectory() + "/" + selectedText());
        if (tempfile.exists()) {
            menu.addAction(tr("&Open"), this, [this] {
                QString file = workingDirectory() + "/" + selectedText();
                QString cmd = QString("xdg-open ") + file;
                //在linux下，可以通过system来xdg-open命令调用默认程序打开文件；
                system(cmd.toStdString().c_str());
                // qDebug() << file << " open";
            });
        }
    }
    /********************* Modify by n014361 wangpeili End ************************/

    menu.addAction(tr("&Open file manager"), this, [this] {
        DDesktopServices::showFolder(QUrl::fromLocalFile(workingDirectory()));
    });

    menu.addSeparator();

    menu.addAction(tr("&Horizontal split"), this, [this] {
        parentPage()->split(Qt::Horizontal);
    });

    menu.addAction(tr("&Vertical split"), this, [this] {
        parentPage()->split(Qt::Vertical);
    });

    /******** Modify by n014361 wangpeili 2020-02-21: 增加关闭窗口和关闭其它窗口菜单    ****************/
    menu.addAction(tr("Close &window"), this, [this] {
        parentPage()->closeSplit(parentPage()->currentTerminal());
    });
    //menu.addAction(tr("Close &Window"), this, [this] { ((TermWidgetPage *)m_Page)->close();});
    if (parentPage()->getTerminalCount() > 1) {
        menu.addAction(tr("Close &other &window"), this, [this] {
            parentPage()->closeOtherTerminal();
        });
    };

    /********************* Modify by n014361 wangpeili End ************************/
    menu.addSeparator();
    menu.addAction(tr("New &workspace"), this, [this] {
        parentPage()->parentMainWindow()->createNewWorkspace();
    });

    menu.addSeparator();

    if (!parentPage()->parentMainWindow()->isQuakeMode()) {
        bool isFullScreen = this->window()->windowState().testFlag(Qt::WindowFullScreen);
        if (isFullScreen) {
            menu.addAction(tr("Exit full&screen"), this, [this] {
                parentPage()->parentMainWindow()->switchFullscreen();
            });
        } else {
            menu.addAction(tr("Full&screen"), this, [this] {
                parentPage()->parentMainWindow()->switchFullscreen();
            });
        }
    }

    menu.addAction(tr("&Find"), this, [this] {
        parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_SEARCHBAR);
    });
    menu.addSeparator();

    if (!selectedText().isEmpty()) {
        DMenu *search = new DMenu(tr("&Search"), this);

        search->addAction("Bing", this, [this] {
            QString strurl = "https://cn.bing.com/search?q=" + selectedText();
            QDesktopServices::openUrl(QUrl(strurl));
        });
        search->addAction("Baidu", this, [this] {
            QString strurl = "https://www.baidu.com/s?wd=" + selectedText();
            QDesktopServices::openUrl(QUrl(strurl));
        });
        search->addAction("Github", this, [this] {
            QString strurl = "https://github.com/search?q=" + selectedText();
            QDesktopServices::openUrl(QUrl(strurl));
        });
        search->addAction("Stack Overflow", this, [this] {
            QString strurl = "https://stackoverflow.com/search?q=" + selectedText();
            QDesktopServices::openUrl(QUrl(strurl));
        });
        menu.addMenu(search);
        menu.addSeparator();
    }

    menu.addAction(tr("Rename title"), this, [this] {
        //QString currTabTitle = this->property("currTabTitle").toString();
        /************************ Mod by sunchengxi 2020-04-30:分屏修改标题异常问题 Begin************************/
        QString currTabTitle = parentPage()->parentMainWindow()->getCurrTabTitle();
        if (currTabTitle.isEmpty())
        {
            currTabTitle = this->property("currTabTitle").toString();
        }
        /************************ Mod by sunchengxi 2020-04-30:分屏修改标题异常问题 End  ************************/
        if (currTabTitle.isNull())
        {
            currTabTitle = this->title();
        }
        qDebug() << "currTabTitle" << currTabTitle << endl;
        Utils::showRenameTitleDialog(currTabTitle, this);
    });

    menu.addAction(tr("&Encoding"), this, [this] {
        parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_ENCODING);
    });

    menu.addAction(tr("Custom commands"), this, [this] {
        parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_CUSTOMCOMMAND);
    });

    menu.addAction(tr("Remote management"), this, [this] {
        parentPage()->parentMainWindow()->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);
    });

    if (isInRemoteServer()) {
        menu.addSeparator();
        menu.addAction(tr("Upload file"), this, [this] {
            parentPage()->parentMainWindow()->remoteUploadFile();
        });
        menu.addAction(tr("Download file"), this, [this] {
            parentPage()->parentMainWindow()->remoteDownloadFile();
        });
    }

    menu.addSeparator();

    menu.addAction(tr("Settings"), this, [this] {
        parentPage()->parentMainWindow()->showSettingDialog();
    });

    // display the menu.
    menu.exec(mapToGlobal(pos));
}

bool TermWidget::isInRemoteServer()
{
    int pid = getForegroundProcessId();
    if (pid <= 0) {
        return false;
    }
    QString pidFilepath = "/proc/" + QString::number(pid) + "/comm";
    QFile pidFile(pidFilepath);
    if (pidFile.exists()) {
        pidFile.open(QIODevice::ReadOnly | QIODevice::Text);
        QString commString(pidFile.readLine());
        pidFile.close();
        if ("expect" == commString.trimmed()) {
            return true;
        }
    }
    return false;
}

void TermWidget::setTermOpacity(qreal opacity)
{
    setTerminalOpacity(opacity);
    /******* Modify by n014361 wangpeili 2020-01-04: 修正实时设置透明度问题************/
    hide();
    show();
    /********************* Modify by n014361 wangpeili End ************************/
}

/*******************************************************************************
 1. @函数:   void TermWidgetWrapper::setTerminalFont(const QString &fontName)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     设置字体
*******************************************************************************/
void TermWidget::setTermFont(const QString &fontName)
{
    QFont font = getTerminalFont();
    font.setFamily(fontName);
    setTerminalFont(font);
}

/*******************************************************************************
 1. @函数:   void TermWidgetWrapper::setTerminalFontSize(const int fontSize)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     设置字体大小
*******************************************************************************/
void TermWidget::setTermFontSize(const int fontSize)
{
    QFont font = getTerminalFont();
    font.setFixedPitch(true);
    font.setPointSize(fontSize);
    setTerminalFont(font);
}


/*******************************************************************************
 1. @函数:   void TermWidgetWrapper::skipToNextCommand()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:    跳转到下一个命令（这个功能没找到库的接口，现在是暂时是以虚拟键形式实现）
*******************************************************************************/
void TermWidget::skipToNextCommand()
{
    qDebug() << "skipToNextCommand";
}

/*******************************************************************************
 1. @函数:  void TermWidgetWrapper::skipToPreCommand()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:   跳转到前一个命令（这个功能没找到库的接口，现在是暂时是以虚拟键形式实现）
*******************************************************************************/
void TermWidget::skipToPreCommand()
{
    qDebug() << "skipToPreCommand";
}

/*******************************************************************************
 1. @函数:  void TermWidgetWrapper::setCursorShape(int shape)
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     设置光标形状
*******************************************************************************/
void TermWidget::setCursorShape(int shape)
{
    Konsole::Emulation::KeyboardCursorShape cursorShape = Konsole::Emulation::KeyboardCursorShape(shape);
    setKeyboardCursorShape(cursorShape);
}

void TermWidget::setPressingScroll(bool enable)
{
    if (enable) {
        setMotionAfterPasting(2);
    } else {
        setMotionAfterPasting(0);
    }
}

bool TermWidget::safeClose()
{
    if (hasRunningProcess()) {
        return Utils::showExitConfirmDialog(this);
    }
    return  true;
}

/*******************************************************************************
 1. @函数:   void TermWidgetWrapper::pasteSelection()
 2. @作者:     n014361 王培利
 3. @日期:     2020-01-10
 4. @说明:     粘贴选择内容
*******************************************************************************/
void TermWidget::wpasteSelection()
{
    int x1, y1, x2, y2;
    getSelectionStart(x1, y1);
    qDebug() << x1 << y1;
    getSelectionEnd(x2, y2);
    qDebug() << x2 << y2;

    pasteSelection();
}

/*******************************************************************************
 1. @函数:    onSettingValueChanged
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-20
 4. @说明:    Terminal的各项设置生效
*******************************************************************************/
void TermWidget::onSettingValueChanged(const QString &keyName)
{
    qDebug() << "onSettingValueChanged:" << keyName;
    if (keyName == "basic.interface.opacity") {
        setTermOpacity(Settings::instance()->opacity());
        return;
    }

    if (keyName == "basic.interface.font") {
        setTermFont(Settings::instance()->fontName());
        return;
    }

    if (keyName == "basic.interface.font_size") {
        setTermFontSize(Settings::instance()->fontSize());
        return;
    }

    if (keyName == "advanced.cursor.cursor_shape") {
        setCursorShape(Settings::instance()->cursorShape());
        return;
    }

    if (keyName == "advanced.cursor.cursor_blink") {
        setBlinkingCursor(Settings::instance()->cursorBlink());
        return;
    }

    if (keyName == "advanced.scroll.scroll_on_key") {
        setPressingScroll(Settings::instance()->PressingScroll());
        return;
    }

    if (keyName == "basic.interface.theme") {
        return;
    }

    if (keyName == "basic.interface.encoding") {
        setTextCodec(QTextCodec::codecForName(Settings::instance()->encoding().toUtf8()));
        return;
    }
    // 这里只是立即生效一次，真正生效起作用的地方在初始的connect中
    if (keyName == "advanced.cursor.auto_copy_selection") {
        if (Settings::instance()->IsPasteSelection()) {
            copyClipboard();
        } else {
            QApplication::clipboard()->clear(QClipboard::Clipboard);
        }

        return;
    }

    if (keyName == "advanced.scroll.scroll_on_output") {
        qDebug() << "settingValue[" << keyName << "] changed to " << Settings::instance()->OutputtingScroll()
                 << ", auto effective when happen";
        return;
    }

    qDebug() << "settingValue[" << keyName << "] changed is not effective";
}

