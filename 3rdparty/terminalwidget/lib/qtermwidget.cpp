/*  Copyright (C) 2008 e_k (e_k@users.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <QLayout>
#include <QBoxLayout>
#include <QtDebug>
#include <QDir>
#include <QMessageBox>
#include <QApplication>

#include "ColorTables.h"
#include "Session.h"
#include "SessionManager.h"
#include "Screen.h"
#include "ScreenWindow.h"
#include "Emulation.h"
#include "TerminalDisplay.h"
#include "KeyboardTranslator.h"
#include "ColorScheme.h"
#include "SearchBar.h"
#include "qtermwidget.h"
#include "history/compact/CompactHistoryType.h"
#include "history/HistoryTypeFile.h"

#ifdef Q_OS_MACOS
// Qt does not support fontconfig on macOS, so we need to use a "real" font name.
#define DEFAULT_FONT_FAMILY                   "Menlo"
#else
#define DEFAULT_FONT_FAMILY                   "Monospace"
#endif

#define STEP_ZOOM 1

using namespace Konsole;

// 翻译文件加载
QTranslator *QTermWidget::m_translator = nullptr;

void *createTermWidget(int startnow, void *parent)
{
    return (void *) new QTermWidget(startnow, (QWidget *)parent);
}

struct TermWidgetImpl {
    explicit TermWidgetImpl(QWidget *parent = 0);

    TerminalDisplay *m_terminalDisplay;
    Session *m_session;

    Session *createSession(QWidget *parent);
    TerminalDisplay *createTerminalDisplay(Session *session, QWidget *parent);
};

TermWidgetImpl::TermWidgetImpl(QWidget *parent)
{
    this->m_session = createSession(parent);
    SessionManager::instance()->saveSession(this->m_session);

    this->m_terminalDisplay = createTerminalDisplay(this->m_session, parent);
    this->m_terminalDisplay->setSessionId(this->m_session->sessionId());
}

Session *TermWidgetImpl::createSession(QWidget *parent)
{
    Session *session = new Session(parent);

    session->setTitle(Session::NameRole, QLatin1String("Terminal"));

    /* Thats a freaking bad idea!!!!
     * /bin/bash is not there on every system
     * better set it to the current $SHELL
     * Maybe you can also make a list available and then let the widget-owner decide what to use.
     * By setting it to $SHELL right away we actually make the first filecheck obsolete.
     * But as iam not sure if you want to do anything else ill just let both checks in and set this to $SHELL anyway.
     */
    //session->setProgram("/bin/bash");

    session->setProgram(QString::fromLocal8Bit(qgetenv("SHELL")));

    QStringList args = QStringList(QString());
    session->setArguments(args);
    session->setAutoClose(true);

    session->setCodec(QTextCodec::codecForName("UTF-8"));

    session->setFlowControlEnabled(true);
    session->setHistoryType(CompactHistoryType(10000));

    session->setDarkBackground(true);

    session->setKeyBindings(QString());
    return session;
}

TerminalDisplay *TermWidgetImpl::createTerminalDisplay(Session *session, QWidget *parent)
{
    //TerminalDisplay* display = new TerminalDisplay(this);
    TerminalDisplay *display = new TerminalScreen(parent);

    display->setBellMode(TerminalDisplay::NotifyBell);
    display->setTerminalSizeHint(true);
    display->setTripleClickMode(TerminalDisplay::SelectWholeLine);
    display->setTerminalSizeStartup(true);
    display->setHideCursor(true);

    display->setRandomSeed(session->sessionId() * 31);

    return display;
}

QTermWidget::QTermWidget(int startnow, QWidget *parent) : QWidget(parent)
{
    init(startnow);
}

QTermWidget::QTermWidget(QWidget *parent) : QWidget(parent)
{
    init(1);
}

void QTermWidget::selectionChanged(bool textSelected)
{
    emit copyAvailable(textSelected);
}

void QTermWidget::search(QString txt, bool forwards, bool next)
{
    /***mod begin by ut001121 zhangmeng 20200515 修复BUG22626***/
    int startColumn, startLine;

    if (m_bHasSelect) {
        /***mod begin by ut001121 zhangmeng 20200814 修复BUG40895***/
        if (next) {
            startColumn = m_endColumn + 1;
            startLine = m_endLine;
        } else {
            if (m_startColumn == 0) {
                startColumn = -1;
                startLine = m_startLine > 0 ? m_startLine - 1 : m_impl->m_session->emulation()->lineCount();
            } else {
                startColumn = m_startColumn;
                startLine = m_startLine;
            }
        }
        /***mod end by ut001121 zhangmeng 20200814***/
    } else if (next) { // search from just after current selection
        m_impl->m_terminalDisplay->screenWindow()->screen()->getSelectionEnd(startColumn, startLine);
        startColumn++;
    } else { // search from start of current selection
        m_impl->m_terminalDisplay->screenWindow()->screen()->getSelectionStart(startColumn, startLine);
    }

    qDebug() << "current selection starts at: " << startColumn << startLine;
    qDebug() << "current cursor position: " << m_impl->m_terminalDisplay->screenWindow()->cursorPosition();
    qDebug() << "current backwardsPosition" << m_lastBackwardsPosition << endl;

    QString searchText(txt);
    // qDebug() << "regExp??????" << regExp.isEmpty();
    //regExp.setPatternSyntax(QRegExp::FixedString);
    //regExp.setCaseSensitivity(Qt::CaseSensitive);

    HistorySearch *historySearch =
        new HistorySearch(m_impl->m_session->emulation(), searchText, forwards, m_isLastForwards, startColumn, startLine, this);
    connect(historySearch, SIGNAL(matchFound(int, int, int, int, int, int, int)), this, SLOT(matchFound(int, int, int, int, int, int, int)));
    connect(this, SIGNAL(sig_noMatchFound()), this, SLOT(clearSelection()));

    connect(historySearch, &HistorySearch::noMatchFound, this, [this]() { emit sig_noMatchFound(); });
    // connect(historySearch, SIGNAL(noMatchFound()), m_searchBar, SLOT(noMatchFound()));
    historySearch->search(m_lastBackwardsPosition, m_startColumn, m_startLine);
    /***mod end by ut001121***/

    m_isLastForwards = forwards;
}

void QTermWidget::matchFound(int startColumn, int startLine, int endColumn, int endLine, int lastBackwardsPosition, int loseChinese, int matchChinese)
{
    /***mod begin by ut001121 zhangmeng 20200515 修复BUG22626***/
    m_bHasSelect = true;
    m_startColumn = startColumn;
    m_startLine = startLine;
    m_endColumn = endColumn;
    m_endLine = endLine;
    if (lastBackwardsPosition != -1) {
        m_lastBackwardsPosition = lastBackwardsPosition;
    }

    ScreenWindow *sw = m_impl->m_terminalDisplay->screenWindow();
    qDebug() << "Scroll to" << startLine;
    sw->scrollTo(startLine);
    sw->setTrackOutput(false);
    sw->setSelectionStart(startColumn + loseChinese, startLine - sw->currentLine(), false);
    sw->setSelectionEnd(endColumn + matchChinese, endLine - sw->currentLine());
    sw->notifyOutputChanged();
    /***mod end by ut001121***/
    // 结束查找
    emit sig_matchFound();
}

void QTermWidget::clearSelection()
{
    /***add by ut001121 zhangmeng 20200515 修复BUG22626***/
    m_bHasSelect = false;
    m_impl->m_terminalDisplay->screenWindow()->clearSelection();
}

void QTermWidget::noMatchFound()
{
    /***add by ut001121 zhangmeng 20200515 修复BUG22626***/
    m_bHasSelect = false;
    m_impl->m_terminalDisplay->screenWindow()->clearSelection();
}

int QTermWidget::getShellPID()
{
    return m_impl->m_session->processId();
}

int QTermWidget::getSessionId()
{
    int currSessionId = m_impl->m_session->sessionId();
    return currSessionId;
}

void QTermWidget::snapshot()
{
    Q_ASSERT(m_impl->m_session != nullptr);

    Session *currSession = m_impl->m_session;
    // use process name as tab title, which not for display
    QString title = currSession->getDynamicProcessName();
    title = title.simplified();

    // use the fallback title if needed
    if (title.isEmpty()) {
        title = currSession->title(Session::NameRole);
    }

    // apply new title
    currSession->setTitle(Session::DisplayedTitleRole, title);

    if (currSession->isForegroundProcessActive()) {
        emit isTermIdle(false);
    } else {
        emit isTermIdle(true);
    }
}

void QTermWidget::changeDir(const QString &dir)
{
    /*
       this is a very hackish way of trying to determine if the shell is in
       the foreground before attempting to change the directory.  It may not
       be portable to anything other than Linux.
    */
    QString strCmd;
    strCmd.setNum(getShellPID());
    strCmd.prepend(QLatin1String("ps -j "));
    strCmd.append(QLatin1String(" | tail -1 | awk '{ print $5 }' | grep -q \\+"));
    int retval = system(strCmd.toStdString().c_str());

    if (!retval) {
        QString cmd = QLatin1String("cd ") + dir + QLatin1Char('\n');
        sendText(cmd);
    }
}

QSize QTermWidget::sizeHint() const
{
    QSize size = m_impl->m_terminalDisplay->sizeHint();
    size.rheight() = 150;
    return size;
}

void QTermWidget::setTerminalSizeHint(bool on)
{
    m_impl->m_terminalDisplay->setTerminalSizeHint(on);
}

bool QTermWidget::terminalSizeHint()
{
    return m_impl->m_terminalDisplay->terminalSizeHint();
}

void QTermWidget::startShellProgram()
{
    if (m_impl->m_session->isRunning()) {
        return;
    }

    m_impl->m_session->run();

    // add snapshot timer when session start running
    addSnapShotTimer();
}

// take a snapshot of the session state every so often when
// user activity occurs
void QTermWidget::addSnapShotTimer()
{
    // the timer is owned by the session so that it will be destroyed along
    // with the session
    Session *currSession = m_impl->m_session;
    m_interactionTimer = new QTimer(currSession);
    m_interactionTimer->setSingleShot(true);
    m_interactionTimer->setInterval(500);
    m_termDisplay = m_impl->m_terminalDisplay;
    connect(m_interactionTimer, &QTimer::timeout, this, &QTermWidget::snapshot);
    connect(m_termDisplay.data(), &Konsole::TerminalDisplay::keyPressedSignal, this, &QTermWidget::interactionHandler);

    // take a snapshot of the session state periodically in the background
    auto backgroundTimer = new QTimer(currSession);
    backgroundTimer->setSingleShot(false);
    backgroundTimer->setInterval(2000);
    connect(backgroundTimer, &QTimer::timeout, this, &QTermWidget::snapshot);
    backgroundTimer->start();
}

void QTermWidget::interactionHandler()
{
    m_interactionTimer->start();
}

/*******************************************************************************
 1. @函数:    setisAllowScroll
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-09-14
 4. @说明:    设置是否允许滚动到最新的位置
 当有输出且并不是在setZoom之后,此标志为true 允许滚动
 当有输出且在setZoom之后,比标志位false 不允许滚动
*******************************************************************************/
void QTermWidget::setIsAllowScroll(bool isAllowScroll)
{
    m_impl->m_terminalDisplay->setIsAllowScroll(isAllowScroll);
}

/*******************************************************************************
 1. @函数:    setNoHasSelect
 2. @作者:    ut001000 任飞翔
 3. @日期:    2020-12-02
 4. @说明:    当搜索框出现时，设置m_bHasSelect为false,避免搜索框隐藏再显示之后，继续走m_bHasSelect为true流程，导致崩溃
*******************************************************************************/
void QTermWidget::setNoHasSelect()
{
    if (m_bHasSelect) {
        //清除搜索状态
        m_lastBackwardsPosition = -1;
        m_isLastForwards = false;
        m_startColumn = 0;
        m_startLine = 0;
        m_endColumn = 0;
        m_endLine = 0;
    }

    m_bHasSelect = false;
}

/*******************************************************************************
 1. @函数:    getisAllowScroll
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-09-14
 4. @说明:    获取是否允许输出时滚动
*******************************************************************************/
bool QTermWidget::getIsAllowScroll() const
{
    return m_impl->m_terminalDisplay->getIsAllowScroll();
}

void QTermWidget::startTerminalTeletype()
{
    if (m_impl->m_session->isRunning()) {
        return;
    }

    m_impl->m_session->runEmptyPTY();
    // redirect data from TTY to external recipient
    connect(
        m_impl->m_session->emulation(), SIGNAL(sendData(const char *, int, const QTextCodec *)), this, SIGNAL(sendData(const char *, int, const QTextCodec *)));
}

void QTermWidget::init(int startnow)
{
    m_layout = new QVBoxLayout();
    m_layout->setMargin(0);
    setLayout(m_layout);

    // translations
    // First check $XDG_DATA_DIRS. This follows the implementation in libqtxdg
    QString d = QFile::decodeName(qgetenv("XDG_DATA_DIRS"));
#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
    QStringList dirs = d.split(QLatin1Char(':'), Qt::SkipEmptyParts);
#else
    QStringList dirs = d.split(QLatin1Char(':'), QString::SkipEmptyParts);
#endif
    if (dirs.isEmpty()) {
        dirs.append(QString::fromLatin1("/usr/local/share"));
        dirs.append(QString::fromLatin1("/usr/share"));
    }
    dirs.append(QFile::decodeName(TRANSLATIONS_DIR));

    // 语言，整个应用程序只需要设置一次
    if (nullptr == m_translator) {
        // 绑定的父类需要是全局的，不能是this防止窗口被关闭后翻译也随之消失
        m_translator = new QTranslator(qApp);
        for (const QString &dir : qAsConst(dirs)) {
            qDebug() << "Trying to load translation file from dir" << dir;
            if (m_translator->load(
                        QLocale::system(), QLatin1String("terminalwidget"), QLatin1String(QLatin1String("_")), dir)) {
                qApp->installTranslator(m_translator);
                qDebug() << "Translations found in" << dir;
                break;
            }
        }
    }

    m_impl = new TermWidgetImpl(this);
    m_layout->addWidget(m_impl->m_terminalDisplay);

    connect(m_impl->m_session, SIGNAL(bellRequest(QString)), m_impl->m_terminalDisplay, SLOT(bell(QString)));
    connect(m_impl->m_terminalDisplay, SIGNAL(notifyBell(QString)), this, SIGNAL(bell(QString)));

    connect(m_impl->m_session, SIGNAL(activity()), this, SIGNAL(activity()));
    connect(m_impl->m_session, SIGNAL(silence()), this, SIGNAL(silence()));
    connect(m_impl->m_session, &Session::profileChangeCommandReceived, this, &QTermWidget::profileChanged);
    connect(m_impl->m_session, &Session::receivedData, this, &QTermWidget::receivedData);
    connect(m_impl->m_session, &Session::started, this, &QTermWidget::processStarted);
    // 标签标题参数变化
    connect(m_impl->m_session, &Session::titleArgsChange, this, &QTermWidget::titleArgsChange);
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/
    // 用于卸载终端弹出框提示
    connect(m_impl->m_session, SIGNAL(sessionUninstallTerminal(QString)), this, SIGNAL(uninstallTerminal(QString)));
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/

    connect(m_impl->m_session, &Session::titleChanged, this, [ = ] {
        m_impl->m_terminalDisplay->setHideCursor(false);
    });

    // That's OK, FilterChain's dtor takes care of UrlFilter.
    UrlFilter *urlFilter = new UrlFilter();
    connect(urlFilter, &UrlFilter::activated, this, &QTermWidget::urlActivated);
    m_impl->m_terminalDisplay->filterChain()->addFilter(urlFilter);
    m_impl->m_terminalDisplay->filterChain()->setSessionId(m_impl->m_session->sessionId());


    if (startnow && m_impl->m_session) {
        m_impl->m_session->run();
    }

    this->setFocus(Qt::OtherFocusReason);
    this->setFocusPolicy(Qt::WheelFocus);
    m_impl->m_terminalDisplay->resize(this->size());

    this->setFocusProxy(m_impl->m_terminalDisplay);
    connect(m_impl->m_terminalDisplay, SIGNAL(copyAvailable(bool)), this, SLOT(selectionChanged(bool)));
    connect(m_impl->m_terminalDisplay, SIGNAL(termGetFocus()), this, SIGNAL(termGetFocus()));
    connect(m_impl->m_terminalDisplay, SIGNAL(leftMouseClick()), this, SIGNAL(leftMouseClick()));
    connect(m_impl->m_terminalDisplay, SIGNAL(termLostFocus()), this, SIGNAL(termLostFocus()));
    // 将拖拽进入的urls名返回给终端 将要转发给Emu的数据
    connect(m_impl->m_terminalDisplay, SIGNAL(sendStringToEmu(const char *)), this, SIGNAL(sendUrlsToTerm(const char *)));
    connect(
        m_impl->m_terminalDisplay, SIGNAL(keyPressedSignal(QKeyEvent *)), this, SIGNAL(termKeyPressed(QKeyEvent *)));
    //    m_impl->m_terminalDisplay->setSize(80, 40);

    QFont font = QApplication::font();
    font.setFamily(QLatin1String(DEFAULT_FONT_FAMILY));
    font.setPointSize(10);
    font.setStyleHint(QFont::TypeWriter);
    setTerminalFont(font);
//    m_searchBar->setFont(font);

    setScrollBarPosition(NoScrollBar);
    setKeyboardCursorShape(Emulation::KeyboardCursorShape::BlockCursor);

    m_impl->m_session->addView(m_impl->m_terminalDisplay);

    connect(m_impl->m_session, SIGNAL(resizeRequest(QSize)), this, SLOT(setSize(QSize)));
    connect(m_impl->m_session, SIGNAL(finished()), this, SLOT(sessionFinished()));
    connect(m_impl->m_session, &Session::titleChanged, this, &QTermWidget::titleChanged);
    connect(m_impl->m_session, &Session::cursorChanged, this, &QTermWidget::cursorChanged);
    connect(m_impl->m_session, &Session::shellWarningMessage, this, &QTermWidget::shellWarningMessage);

    //将终端活动状态传给SessionManager单例
    connect(this, SIGNAL(isTermIdle(bool)), SessionManager::instance(), SIGNAL(sessionIdle(bool)));
}

QTermWidget::~QTermWidget()
{
    SessionManager::instance()->removeSession(m_impl->m_session->sessionId());

    delete m_impl;
    emit destroyed();
}

QList<int> QTermWidget::getRunningSessionIdList()
{
    SessionManager *sessionMgr = SessionManager::instance();
    QList<Session *> sessionList = sessionMgr->sessions();

    QList<int> sessionIdList;
    for (int i = 0; i < sessionList.size(); i++) {
        Session *session = sessionList.at(i);
        if ((session == nullptr) || !session->isForegroundProcessActive()) {
            continue;
        }
        sessionIdList.append(session->sessionId());
    }

    return sessionIdList;
}

bool QTermWidget::hasRunningProcess()
{
    QList<int> sessionIdList = getRunningSessionIdList();
    for (int i = 0; i < sessionIdList.size(); i++) {
        int sessionId = sessionIdList.at(i);
        int currSessionId = m_impl->m_session->sessionId();
        if (sessionId == currSessionId) {
            return true;
        }
    }

    return false;
}

void QTermWidget::setTerminalFont(const QFont &font)
{
    m_impl->m_terminalDisplay->setVTFont(font);
}

QFont QTermWidget::getTerminalFont()
{
    return m_impl->m_terminalDisplay->getVTFont();
}

void QTermWidget::setTerminalOpacity(qreal level)
{
    m_impl->m_terminalDisplay->setOpacity(level);
}

void QTermWidget::setTerminalBackgroundImage(QString backgroundImage)
{
    m_impl->m_terminalDisplay->setBackgroundImage(backgroundImage);
}

void QTermWidget::setShellProgram(const QString &progname)
{
    if (!m_impl->m_session)
        return;
    m_impl->m_session->setProgram(progname);
    qDebug() << "set Program" << progname;
}

void QTermWidget::setWorkingDirectory(const QString &dir)
{
    if (!m_impl->m_session)
        return;
    m_impl->m_session->setInitialWorkingDirectory(dir);
}

QString QTermWidget::workingDirectory()
{
    if (!m_impl->m_session)
        return QString();

#ifdef Q_OS_LINUX
    // Christian Surlykke: On linux we could look at /proc/<pid>/cwd which should be a link to current
    // working directory (<pid>: process id of the shell). I don't know about BSD.
    // Maybe we could just offer it when running linux, for a start.
    QDir d(QString::fromLatin1("/proc/%1/cwd").arg(getShellPID()));
    if (!d.exists()) {
        qDebug() << "Cannot find" << d.dirName();
        goto fallback;
    }
    return d.canonicalPath();

fallback:
#endif
    // fallback, initial WD
    return m_impl->m_session->initialWorkingDirectory();
}

void QTermWidget::setArgs(const QStringList &args)
{
    if (!m_impl->m_session)
        return;
    m_impl->m_session->setArguments(args);
    qDebug() << "set Arguments" << args;
}

void QTermWidget::setTextCodec(QTextCodec *codec)
{
    if (!m_impl->m_session)
        return;
    m_impl->m_session->setCodec(codec);
}

void QTermWidget::setTerminalWordCharacters(const QString &wc)
{
    m_impl->m_terminalDisplay->setWordCharacters(wc);
}

/*******************************************************************************
 1. @函数:    setColorScheme
 2. @作者:    ut000125 sunchengxi
 3. @日期:    2020-12-01
 4. @说明:    设置主题的配色方案,根据参数 needReloadTheme 判断是否需要重新加载
*******************************************************************************/
int QTermWidget::setColorScheme(const QString &origName)
{
    const ColorScheme *cs = nullptr;

    const bool isFile = QFile::exists(origName);
    const QString &name = isFile ? QFileInfo(origName).baseName() : origName;

    // avoid legacy (int) solution
    if (!availableColorSchemes().contains(name)) {
        if (isFile) {
            if (ColorSchemeManager::instance()->loadCustomColorScheme(origName)) {
                cs = ColorSchemeManager::instance()->findColorScheme(name);
            } else {
                qWarning() << Q_FUNC_INFO << "cannot load color scheme from" << origName;
            }
        }
        if (!cs) {
            cs = ColorSchemeManager::instance()->defaultColorScheme();
        }
    } else {
        if (name == "customTheme") {
            ColorSchemeManager::instance()->reloadColorScheme(origName);
        }
        cs = ColorSchemeManager::instance()->findColorScheme(name);
    }

    if (!cs) {
        QMessageBox::information(this, tr("Color Scheme Error"), tr("Cannot load color scheme: %1").arg(name));
        return -1;
    }
    ColorEntry table[TABLE_COLORS];
    cs->getColorTable(table);
    m_impl->m_terminalDisplay->setColorTable(table);
    const QColor &background_color = table[DEFAULT_BACK_COLOR].color;
    return background_color.lightness();
}

QStringList QTermWidget::availableColorSchemes()
{
    QStringList ret;
    const auto allColorSchemes = ColorSchemeManager::instance()->allColorSchemes();
    for (const ColorScheme *cs : allColorSchemes)
        ret.append(cs->name());
    return ret;
}

void QTermWidget::addCustomColorSchemeDir(const QString &custom_dir)
{
    ColorSchemeManager::instance()->addCustomColorSchemeDir(custom_dir);
}

void QTermWidget::setSize(const QSize &size)
{
    m_impl->m_terminalDisplay->setSize(size.width(), size.height());
}

void QTermWidget::setHistorySize(int lines)
{
    if (lines < 0)
        m_impl->m_session->setHistoryType(HistoryTypeFile());
    else
        m_impl->m_session->setHistoryType(CompactHistoryType(lines));
}

void QTermWidget::setScrollBarPosition(ScrollBarPosition pos)
{
    m_impl->m_terminalDisplay->setScrollBarPosition(pos);
}

void QTermWidget::scrollToEnd()
{
    m_impl->m_terminalDisplay->scrollToEnd();
}
/*******************************************************************************
 1. @函数:    setTrackOutput
 2. @作者:    n014361 王培利
 3. @日期:    2020-02-17
 4. @说明:    设置当前屏幕是否滚动
*******************************************************************************/
void QTermWidget::setTrackOutput(bool enable)
{
    m_impl->m_terminalDisplay->screenWindow()->setTrackOutput(enable);
}

void QTermWidget::sendText(const QString &text)
{
    //标记当前命令是代码中通过sendText发给终端的(而不是用户手动输入的命令)
    bool isSendByRemoteManage = this->property("isSendByRemoteManage").toBool();
    if (isSendByRemoteManage) {
        //将isSendByRemoteManage标记同步给Session
        m_impl->m_session->setProperty("isSendByRemoteManage", QVariant(true));

        //立即修改回false，防止误认其他命令
        this->setProperty("isSendByRemoteManage", QVariant(false));
    }

    m_impl->m_session->sendText(text);

    //标记后立即修改回false，防止误认其他命令
    m_impl->m_session->setProperty("isSendByRemoteManage", QVariant(false));
}

void QTermWidget::sendKeyEvent(QKeyEvent *e)
{
    m_impl->m_session->sendKeyEvent(e);
}

void QTermWidget::resizeEvent(QResizeEvent *)
{
    // qDebug("global window resizing...with %d %d", this->size().width(), this->size().height());
    m_impl->m_terminalDisplay->resize(this->size());
}

void QTermWidget::sessionFinished()
{
    emit finished();
}

void QTermWidget::bracketText(QString &text)
{
    m_impl->m_terminalDisplay->bracketText(text);
}

void QTermWidget::copyClipboard()
{
    m_impl->m_terminalDisplay->copyClipboard();
}

void QTermWidget::pasteClipboard()
{
    if (m_impl->m_terminalDisplay->hasFocus()) {
        m_impl->m_terminalDisplay->pasteClipboard();
    }
}

void QTermWidget::pasteSelection()
{
    m_impl->m_terminalDisplay->pasteSelection();
}

void QTermWidget::setZoom(int step)
{
    // 获取字体
    QFont font = m_impl->m_terminalDisplay->getVTFont();

    // 设置字体
    font.setPointSize(font.pointSize() + step);
    setTerminalFont(font);
}

void QTermWidget::zoomIn()
{
    setZoom(STEP_ZOOM);
}

void QTermWidget::zoomOut()
{
    setZoom(-STEP_ZOOM);
}

void QTermWidget::setKeyBindings(const QString &kb)
{
    m_impl->m_session->setKeyBindings(kb);
}

void QTermWidget::clear()
{
    m_impl->m_session->emulation()->reset();
    m_impl->m_session->refresh();
    m_impl->m_session->clearHistory();
}

void QTermWidget::setFlowControlEnabled(bool enabled)
{
    m_impl->m_session->setFlowControlEnabled(enabled);
}

QStringList QTermWidget::availableKeyBindings()
{
    return KeyboardTranslatorManager::instance()->allTranslators();
}

QString QTermWidget::keyBindings()
{
    return m_impl->m_session->keyBindings();
}

void QTermWidget::toggleShowSearchBar()
{
    m_searchBar->isHidden() ? m_searchBar->show() : m_searchBar->hide();
}

bool QTermWidget::flowControlEnabled(void)
{
    return m_impl->m_session->flowControlEnabled();
}

void QTermWidget::setFlowControlWarningEnabled(bool enabled)
{
    if (flowControlEnabled()) {
        // Do not show warning label if flow control is disabled
        m_impl->m_terminalDisplay->setFlowControlWarningEnabled(enabled);
    }
}

void QTermWidget::setEnvironment(const QStringList &environment)
{
    m_impl->m_session->setEnvironment(environment);
}

void QTermWidget::setMotionAfterPasting(int action)
{
    m_impl->m_terminalDisplay->setMotionAfterPasting((Konsole::MotionAfterPasting)action);
}

int QTermWidget::historyLinesCount()
{
    return m_impl->m_terminalDisplay->screenWindow()->screen()->getHistLines();
}

int QTermWidget::screenColumnsCount()
{
    return m_impl->m_terminalDisplay->screenWindow()->screen()->getColumns();
}

int QTermWidget::screenLinesCount()
{
    return m_impl->m_terminalDisplay->screenWindow()->screen()->getLines();
}

void QTermWidget::setSelectionStart(int row, int column)
{
    m_impl->m_terminalDisplay->screenWindow()->screen()->setSelectionStart(column, row, true);
}

void QTermWidget::setSelectionEnd(int row, int column)
{
    m_impl->m_terminalDisplay->screenWindow()->screen()->setSelectionEnd(column, row);
}
/********************************************************************
 1. @函数:    setSelectionAll
 2. @作者:     王培利
 3. @日期:     2020-02-13
 4. @说明:    屏幕全选功能
********************************************************************/
void QTermWidget::setSelectionAll()
{
    m_impl->m_terminalDisplay->setSelectionAll();
}

void QTermWidget::getSelectionStart(int &row, int &column)
{
    m_impl->m_terminalDisplay->screenWindow()->screen()->getSelectionStart(column, row);
}

void QTermWidget::getSelectionEnd(int &row, int &column)
{
    m_impl->m_terminalDisplay->screenWindow()->screen()->getSelectionEnd(column, row);
}

char QTermWidget::getErase()
{
    return m_impl->m_session->getEraseChar();
}

void QTermWidget::setDeleteMode(char *key, int length)
{
    m_impl->m_session->setDeleteMode(key, length);
}

void QTermWidget::setBackspaceMode(char *key, int length)
{
    m_impl->m_session->setBackspaceMode(key, length);
}

QString QTermWidget::selectedText(const Screen::DecodingOptions options)
{
    return m_impl->m_terminalDisplay->screenWindow()->screen()->selectedText(options);
}

void QTermWidget::setMonitorActivity(bool monitor)
{
    m_impl->m_session->setMonitorActivity(monitor);
}

void QTermWidget::setMonitorSilence(bool monitor)
{
    m_impl->m_session->setMonitorSilence(monitor);
}

void QTermWidget::setSilenceTimeout(int seconds)
{
    m_impl->m_session->setMonitorSilenceSeconds(seconds);
}

Filter::HotSpot *QTermWidget::getHotSpotAt(const QPoint &pos) const
{
    int row = 0, column = 0;
    m_impl->m_terminalDisplay->getCharacterPosition(pos, row, column);
    return getHotSpotAt(row, column);
}

Filter::HotSpot *QTermWidget::getHotSpotAt(int row, int column) const
{
    return m_impl->m_terminalDisplay->filterChain()->hotSpotAt(row, column);
}

QList<QAction *> QTermWidget::filterActions(const QPoint &position)
{
    return m_impl->m_terminalDisplay->filterActions(position);
}

int QTermWidget::getPtySlaveFd() const
{
    return m_impl->m_session->getPtySlaveFd();
}

void QTermWidget::setKeyboardCursorShape(KeyboardCursorShape shape)
{
    m_impl->m_terminalDisplay->setKeyboardCursorShape(shape);
}

void QTermWidget::setBlinkingCursor(bool blink)
{
    m_impl->m_terminalDisplay->setBlinkingCursor(blink);
}

void QTermWidget::setBidiEnabled(bool enabled)
{
    m_impl->m_terminalDisplay->setBidiEnabled(enabled);
}

bool QTermWidget::isBidiEnabled()
{
    return m_impl->m_terminalDisplay->isBidiEnabled();
}

QString QTermWidget::title() const
{
    QString title = m_impl->m_session->userTitle();
    if (title.isEmpty())
        title = m_impl->m_session->title(Konsole::Session::NameRole);
    return title;
}

QString QTermWidget::icon() const
{
    QString icon = m_impl->m_session->iconText();
    if (icon.isEmpty())
        icon = m_impl->m_session->iconName();
    return icon;
}

bool QTermWidget::isTitleChanged() const
{
    return m_impl->m_session->isTitleChanged();
}

void QTermWidget::setAutoClose(bool autoClose)
{
    m_impl->m_session->setAutoClose(autoClose);
}

void QTermWidget::cursorChanged(Konsole::Emulation::KeyboardCursorShape cursorShape, bool blinkingCursorEnabled)
{
    // TODO: A switch to enable/disable DECSCUSR?
    setKeyboardCursorShape(cursorShape);
    setBlinkingCursor(blinkingCursorEnabled);
}

void QTermWidget::setMargin(int margin)
{
    m_impl->m_terminalDisplay->setMargin(margin);
}

int QTermWidget::getMargin() const
{
    return m_impl->m_terminalDisplay->margin();
}

void QTermWidget::saveHistory(QIODevice *device)
{
    QTextStream stream(device);
    PlainTextDecoder decoder;
    decoder.begin(&stream);
    m_impl->m_session->emulation()->writeToStream(&decoder, 0, m_impl->m_session->emulation()->lineCount());
}

void QTermWidget::setDrawLineChars(bool drawLineChars)
{
    m_impl->m_terminalDisplay->setDrawLineChars(drawLineChars);
}

void QTermWidget::setBoldIntense(bool boldIntense)
{
    m_impl->m_terminalDisplay->setBoldIntense(boldIntense);
}

int QTermWidget::getForegroundProcessId() const
{
    return m_impl->m_session->foregroundProcessId();
}

QString QTermWidget::getForegroundProcessName() const
{
    return m_impl->m_session->foregroundProcessName();
}

