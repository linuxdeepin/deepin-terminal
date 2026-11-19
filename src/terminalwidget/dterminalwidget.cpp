// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dterminalwidget.h"

#include "dcolorscheme.h"
#include "dhistorysearch.h"
#include "dkeyboardtranslator.h"
#include "dshellsession.h"
#include "dterminalparser.h"
#include "dterminalscreen.h"
#include "dterminalview.h"
#include "dtextfilter.h"

#include <QAction>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QKeyEvent>
#include <QTextCodec>
#include <QVBoxLayout>

#include <functional>

namespace {

// 会话 shell 上下文存取（接入扫描器）
class SessionContextSink final : public DShellContextSink
{
public:
    void saveShellPrompt(int sessionId, const QString &prompt) override
    {
        DSessionManager::instance()->saveShellPrompt(sessionId, prompt);
    }

    QString currentShellPrompt(int sessionId) const override
    {
        return DSessionManager::instance()->currentShellPrompt(sessionId);
    }

    void saveShellCommand(int sessionId, const QString &command) override
    {
        DSessionManager::instance()->saveShellCommand(sessionId, command);
    }
};

constexpr int kZoomStep = 1;

// 键盘布局目录解析：系统安装目录（KB_LAYOUT_DIR）优先，须含至少一个 *.keytab；
// 缺失或为空时回退工程源码数据目录（仅 Debug 构建带 SOURCE_DATA_DIR 宏）。
QString getKbLayoutDir()
{
    QStringList candidates;
    candidates << QStringLiteral(KB_LAYOUT_DIR);
#if defined(SOURCE_DATA_DIR)
    candidates << QStringLiteral(SOURCE_DATA_DIR "/keytabs");
#endif
    for (const QString &candidate : candidates) {
        const QDir dir(candidate);
        if (dir.exists() && !dir.entryList(QStringList() << QStringLiteral("*.keytab"),
                                           QDir::Files).isEmpty()) {
            qDebug() << "Found keyboard layout directory:" << candidate;
            return candidate;
        }
    }
    qDebug() << "Found keyboard layout directory:" << QString();
    return QString();
}

}  // namespace

DTerminalWidget::DTerminalWidget(QWidget *parent)
    : DTerminalDriver(parent)
{
    // 组装组件
    m_session = new DShellSession(this);
    m_view = new DTerminalView(this);
    m_scanner = new DTerminalTextScanner();
    m_scanner->setSessionId(m_session->sessionId());
    m_scanner->setContextSink(new SessionContextSink());

    m_urlFilter = new DUrlFilter();
    m_scanner->addFilter(m_urlFilter);

    // 默认键盘映射
    m_session->setKeymapFile(getKbLayoutDir() + QStringLiteral("/default.keytab"));

    DTerminalParser *parser = m_session->parser();
    DTerminalScreen *screen = parser->primaryScreen();

    // 组合字符序列解析
    const std::function<QString(uint)> sequenceResolver = [](uint hash) {
        ushort length = 0;
        const uint *points = DCharSequenceTable::instance()->lookup(hash, length);
        if (points != nullptr && length > 0)
            return QString::fromUcs4(points, length);
        return QString(QChar(hash));
    };
    screen->setSequenceResolver(sequenceResolver);
    screen->setView(m_view);

    // 屏幕切换
    connect(parser, &DTerminalParser::primaryScreenInUse, this, [this, parser](bool primary) {
        m_view->setScreen(primary ? parser->primaryScreen() : parser->alternateScreen());
        m_view->setAlternateScreen(!primary);
        m_view->updateImage();
    });

    m_view->setScreen(screen);
    m_view->setScanner(m_scanner);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);

    // 焦点代理：上层 TermWidget setFocus() 直接落到视图，
    // 否则首启焦点停在 TermWidget 上，必须再点一次视图才能输入
    setFocusPolicy(Qt::StrongFocus);
    setFocusProxy(m_view);

    wireSignals();

    // 卸载确认：经上层处理器同步询问
    m_session->setUninstallConfirmer([this](const QString &kind) { return confirmUninstall(kind); });
}

DTerminalWidget::~DTerminalWidget()
{
    delete m_scanner;
}

void DTerminalWidget::wireSignals()
{
    // 输出即达时同步窗口位置（不经 10/40ms 节流）。必须接在
    // receivedData → outputReceived 之前：外层"智能滚动"会在
    // outputReceived 时读 atOutputEnd()，视图状态滞后会被误判为
    // "不在底部"而禁用跟随
    connect(m_session, &DShellSession::receivedData, m_view, [this](const QString &) {
        m_view->syncOutputTracking();
    });

    // 会话 → 驱动信号
    connect(m_session, &DShellSession::finished, this, &DTerminalDriver::shellFinished);
    connect(m_session, &DShellSession::receivedData, this, &DTerminalDriver::outputReceived);
    connect(m_session, &DShellSession::titleChanged, this, &DTerminalDriver::titleUpdated);
    connect(m_session, &DShellSession::titleArgsChange, this, &DTerminalDriver::titleArgUpdated);
    connect(m_session, &DShellSession::shellWarningMessage, this, &DTerminalDriver::shellAlert);
    connect(m_session, &DShellSession::osc52ClipboardRequest, this, &DTerminalDriver::clipboardRequestFromShell);

    // 视图 → 驱动信号
    connect(m_view, &DTerminalView::copyAvailable, this, &DTerminalDriver::copyStateChanged);
    connect(m_view, &DTerminalView::focusInTerminal, this, &DTerminalDriver::focusInTerminal);
    connect(m_view, &DTerminalView::focusLeftTerminal, this, &DTerminalDriver::focusLeftTerminal);
    connect(m_view, &DTerminalView::leftMouseClicked, this, &DTerminalDriver::leftMousePressed);

    // URL 过滤 → 驱动信号
    connect(m_urlFilter, &DUrlFilter::activated, this, &DTerminalDriver::urlTriggered);

    // 视图输入 → 会话
    connect(m_view, &DTerminalView::keyPressed, m_session, [this](QKeyEvent *event) {
        m_session->sendKeyEvent(event);
    });
    connect(m_view, &DTerminalView::mouseEvent, this, [this](int button, int column, int line, int eventType) {
        m_session->sendMouseEvent(button, column, line, eventType);
    });
    connect(m_view, &DTerminalView::sendStringToEmulation, this, [this](const char *text) {
        m_session->sendText(QString::fromUtf8(text));
    });

    // Ctrl+左键定位光标：按偏移注入左/右方向键序列
    connect(m_view, &DTerminalView::cursorPositionChanged, this, [this](int count) {
        if (count == 0)
            return;
        const int steps = qAbs(count);
        const int key = count > 0 ? Qt::Key_Right : Qt::Key_Left;
        for (int i = 0; i < steps; ++i) {
            QKeyEvent e(QEvent::KeyPress, key, Qt::NoModifier);
            m_session->sendKeyEvent(&e);
        }
    });

    // 视图尺寸 → 会话
    connect(m_view, &DTerminalView::fontMetricChanged, this, [this](int, int) {
        m_session->setSize(QSize(m_view->columnCount(), m_view->lineCount()));
    });
    connect(m_session, &DShellSession::resizeRequest, this, [this](const QSize &size) {
        m_view->setSize(size.width(), size.height());
        m_view->update();
    });

    // 解析输出 → 视图刷新
    connect(m_session->parser(), &DTerminalParser::outputChanged, m_view, &DTerminalView::notifyOutputChanged);
    connect(m_session->parser(), &DTerminalParser::cursorChanged, m_view, [this](int shape, bool blink) {
        m_view->setCursorShape(static_cast<DTerminalView::DCursorShape>(shape));
        m_view->setBlinkingCursor(blink);
    });
}

void DTerminalWidget::setUninstallHandler(const std::function<bool(const QString &)> &handler)
{
    m_uninstallHandler = handler;
}

bool DTerminalWidget::confirmUninstall(const QString &kind)
{
    if (m_uninstallHandler)
        return m_uninstallHandler(kind);
    return false;
}

QWidget *DTerminalWidget::hostWidget()
{
    return this;
}

QStringList DTerminalWidget::availableColorSchemes()
{
    QStringList names;
    const QList<const DColorScheme *> schemes = DColorSchemeRegistry::instance()->allSchemes();
    for (const DColorScheme *scheme : schemes)
        names << scheme->name();
    return names;
}

// ---- 会话与进程 ----

void DTerminalWidget::startShell()
{
    m_session->runShell();
}

int DTerminalWidget::shellSessionId()
{
    return m_session->sessionId();
}

bool DTerminalWidget::hasLiveProcess()
{
    // hasRunningProcess 语义：仅当前台进程组 ≠ shell 自身
    // （真有命令/应用在前台运行）才算有活动进程；shell 空闲不弹关闭提示
    return m_session->isForegroundProcessActive();
}

int DTerminalWidget::foregroundProcessId()
{
    return m_session->foregroundProcessId();
}

QString DTerminalWidget::shellTitle()
{
    // title 语义：优先 shell 用户标题（OSC 0，
    // fish 的 prompt 用它上报路径），为空时回退 NameRole
    const QString title = m_session->userTitle();
    if (!title.isEmpty())
        return title;
    return m_session->title(DShellSession::NameRole);
}

// ---- 外观与行为配置 ----

void DTerminalWidget::setHistoryLimit(int lines)
{
    // 0 不保留；负数（无限）走文件存储；正数走内存紧凑存储
    if (lines == 0) {
        m_session->setHistoryStorage(DTerminalHistory::StorageKind::None, 0);
    } else if (lines < 0) {
        m_session->setHistoryStorage(DTerminalHistory::StorageKind::File, -1);
    } else {
        m_session->setHistoryStorage(DTerminalHistory::StorageKind::Buffer, lines);
    }
}

void DTerminalWidget::setWordChars(const QString &chars)
{
    m_view->setWordCharacters(chars);
}

void DTerminalWidget::setShellEnv(const QStringList &environment)
{
    m_session->setShellEnvironment(environment);
}

void DTerminalWidget::setShellPath(const QString &program)
{
    m_session->setShellProgram(program);
}

void DTerminalWidget::setShellArgs(const QStringList &arguments)
{
    m_session->setShellArguments(arguments);
}

void DTerminalWidget::setWorkDir(const QString &directory)
{
    m_session->setInitialWorkingDirectory(directory);
}

QString DTerminalWidget::workDir()
{
    return m_session->workingDirectory();
}

void DTerminalWidget::applyOpacity(qreal level)
{
    m_view->setOpacity(level);
}

void DTerminalWidget::applyColorScheme(const QString &name, bool reloadTheme)
{
    DColorSchemeRegistry *registry = DColorSchemeRegistry::instance();

    const bool isFile = QFile::exists(name);
    const QString schemeName = isFile ? QFileInfo(name).baseName() : name;

    const DColorScheme *scheme = registry->findScheme(schemeName);
    if (scheme == nullptr) {
        if (isFile)
            registry->loadCustomScheme(name);
        scheme = registry->findScheme(schemeName);
    }
    if (scheme == nullptr)
        scheme = registry->defaultScheme();
    if (scheme == nullptr)
        return;

    // 自定义主题编辑后按文件路径重读。重读会替换注册表内方案对象，需重新获取指针
    if (schemeName == QLatin1String("customTheme") && reloadTheme) {
        registry->reloadScheme(isFile ? name : schemeName);
        scheme = registry->findScheme(schemeName);
        if (scheme == nullptr)
            scheme = registry->defaultScheme();
        if (scheme == nullptr)
            return;
    }

    DTerminalPaletteEntry table[20];
    scheme->fillPalette(table, m_view->randomSeed());
    m_view->setPalette(table);
    m_session->setDarkBackground(scheme->hasDarkBackground());
}

void DTerminalWidget::setCodec(QTextCodec *codec)
{
    m_session->setCodec(codec);
}

void DTerminalWidget::setAutoClose(bool autoClose)
{
    m_session->setAutoClose(autoClose);
}

QFont DTerminalWidget::terminalFont()
{
    return m_view->terminalFont();
}

void DTerminalWidget::applyFont(const QFont &font)
{
    m_view->setVTFont(font);
}

void DTerminalWidget::setCursorStyle(DCursorStyle style)
{
    m_view->setCursorShape(static_cast<DTerminalView::DCursorShape>(style));
}

void DTerminalWidget::setCursorBlink(bool blink)
{
    m_view->setBlinkingCursor(blink);
}

void DTerminalWidget::enableCursorReposition(bool enable)
{
    m_view->setCursorPositionEnable(enable);
}

void DTerminalWidget::setFlowControl(bool enable)
{
    m_session->setFlowControlEnabled(enable);
}

bool DTerminalWidget::flowControlOn()
{
    return m_session->flowControlEnabled();
}

void DTerminalWidget::setBidi(bool enable)
{
    m_view->setBidiEnabled(enable);
}

void DTerminalWidget::setPasteScrolling(int mode)
{
    m_view->setMotionAfterPasting(mode);
}

// ---- 输入输出 ----

void DTerminalWidget::sendToShell(const QString &text)
{
    m_session->sendText(text);
}

char DTerminalWidget::eraseChar()
{
    return m_session->getEraseChar();
}

void DTerminalWidget::setBackspaceSequence(char *key, int length)
{
    m_session->parser()->setBackspaceSequence(key, length);
}

void DTerminalWidget::setDeleteSequence(char *key, int length)
{
    m_session->parser()->setDeleteSequence(key, length);
}

// ---- 滚动 ----

bool DTerminalWidget::allowScrollOnOutput()
{
    return m_view->isAllowScrollOnOutput();
}

void DTerminalWidget::setAllowScrollOnOutput(bool allow)
{
    m_view->setAllowScrollOnOutput(allow);
}

void DTerminalWidget::setFollowOutput(bool enable)
{
    m_view->setTrackOutput(enable);
}

bool DTerminalWidget::atOutputEnd()
{
    return m_view->atEndOfOutput();
}

// ---- 选区与剪贴板 ----

QString DTerminalWidget::selectionText()
{
    // 保留硬换行，TextPlain 会把行间分隔变成空格
    return m_session->parser()->currentScreen()->selectedText(DTerminalScreen::TextKeepBreaks);
}

void DTerminalWidget::copyToClipboard()
{
    m_view->copyClipboard();
}

void DTerminalWidget::pasteFromClipboard()
{
    m_view->pasteClipboard();
}

void DTerminalWidget::selectAllText()
{
    m_view->setSelectionAll();
}

void DTerminalWidget::resetSelection()
{
    m_hasSelection = false;
    m_session->parser()->currentScreen()->clearSelection();
    m_view->selectionCleared();
}

void DTerminalWidget::resetSelectionGuard()
{
    // 搜索框出现时重置选区状态，避免后续流程误判
    m_hasSelection = false;
}

// ---- 搜索 ----

void DTerminalWidget::searchText(const QString &keyword, bool forwards, bool next)
{
    int startColumn = 0;
    int startLine = 0;

    if (m_hasSelection) {
        if (next) {
            startColumn = m_searchEndColumn + 1;
            startLine = m_searchEndLine;
        } else {
            if (m_searchStartColumn == 0) {
                startColumn = -1;
                startLine = m_searchStartLine > 0
                        ? m_searchStartLine - 1
                        : m_session->parser()->lineCount();
            } else {
                startColumn = m_searchStartColumn;
                startLine = m_searchStartLine;
            }
        }
    } else if (next) {
        m_session->parser()->currentScreen()->getSelectionEnd(startColumn, startLine);
        startColumn++;
    } else {
        m_session->parser()->currentScreen()->getSelectionStart(startColumn, startLine);
    }

    auto *historySearch = new DHistorySearch(m_session->parser()->currentScreen(),
                                             keyword, forwards, m_isLastForwards,
                                             startColumn, startLine, this);
    connect(historySearch, &DHistorySearch::matchFound,
            this, &DTerminalWidget::onMatchFound);
    connect(historySearch, &DHistorySearch::noMatchFound,
            this, &DTerminalWidget::onNoMatchFound);
    historySearch->search(m_lastBackwardsPosition, m_searchStartColumn, m_searchStartLine);

    m_isLastForwards = forwards;
}

void DTerminalWidget::onMatchFound(int startColumn, int startLine, int endColumn, int endLine,
                                   int lastBackwardsPosition, int loseChinese, int matchChinese)
{
    m_hasSelection = true;
    m_searchStartColumn = startColumn;
    m_searchStartLine = startLine;
    m_searchEndColumn = endColumn;
    m_searchEndLine = endLine;
    if (lastBackwardsPosition != -1)
        m_lastBackwardsPosition = lastBackwardsPosition;

    // 滚动到结果处并选中
    m_view->scrollTo(startLine);
    m_view->setTrackOutput(false);
    DTerminalScreen *screen = m_session->parser()->currentScreen();
    screen->setSelectionStart(startColumn + loseChinese, startLine, false);
    screen->setSelectionEnd(endColumn + matchChinese, endLine);
    m_view->notifyOutputChanged();

    emit searchHitFound();
}

void DTerminalWidget::onNoMatchFound()
{
    emit searchHitMissing();
}

// ---- 缩放与热点 ----

void DTerminalWidget::stepZoomIn()
{
    QFont font = m_view->terminalFont();
    font.setPointSize(font.pointSize() + kZoomStep);
    m_view->setVTFont(font);
}

void DTerminalWidget::stepZoomOut()
{
    QFont font = m_view->terminalFont();
    font.setPointSize(font.pointSize() - kZoomStep);
    m_view->setVTFont(font);
}

QList<QAction *> DTerminalWidget::hotspotActions(const QPoint &position)
{
    return m_view->filterActions(position);
}
