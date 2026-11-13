// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dshellsession.h"

#include "dterminalpty.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QTextCodec>

#include <csignal>
#include <pwd.h>
#include <unistd.h>

#include <utility>

// ---- DProcessInfo ----

DProcessInfo::DProcessInfo(int pid)
    : m_pid(pid)
{
}

DProcessInfo::~DProcessInfo() = default;

void DProcessInfo::update()
{
    m_valid = false;
    if (m_pid <= 0)
        return;

    readProcessStat();
    if (!m_valid)
        return;

    readProcessCmdline();
    readProcessCwd();
    if (m_needUserName) {
        struct passwd pwd;
        struct passwd *result = nullptr;
        char buffer[1024];
        // 以 /proc/<pid> 属主推断用户名
        const QString statPath = QStringLiteral("/proc/%1/status").arg(m_pid);
        QFile statusFile(statPath);
        if (statusFile.open(QIODevice::ReadOnly)) {
            QString line;
            while (!(line = QString::fromLocal8Bit(statusFile.readLine())).isEmpty()) {
                if (line.startsWith(QLatin1String("Uid:"))) {
                    const uid_t uid = line.section(QLatin1Char('\t'), 1, 1).toUInt();
                    if (::getpwuid_r(uid, &pwd, buffer, sizeof(buffer), &result) == 0 && result)
                        m_userName = QString::fromLocal8Bit(pwd.pw_name);
                    break;
                }
            }
        }
    }
}

void DProcessInfo::readProcessStat()
{
    const QString statPath = QStringLiteral("/proc/%1/stat").arg(m_pid);
    QFile statFile(statPath);
    if (!statFile.open(QIODevice::ReadOnly))
        return;

    const QByteArray raw = statFile.readLine();
    // pid (comm) state ppid pgrp session tty_nr ...
    int openParen = raw.indexOf('(');
    int closeParen = raw.lastIndexOf(')');
    if (openParen == -1 || closeParen == -1 || closeParen + 1 >= raw.size())
        return;

    m_name = QString::fromLocal8Bit(raw.mid(openParen + 1, closeParen - openParen - 1));

    const QList<QByteArray> fields = raw.mid(closeParen + 2).split(' ');
    if (fields.size() < 4)
        return;

    // fields: state(0) ppid(1) pgrp(2) session(3) tty_nr(4) ...
    m_parentPid = fields.value(1).toInt();
    m_processGroup = fields.value(2).toInt();
    m_session = fields.value(3).toInt();
    m_ttyDevice = fields.value(4).toInt();
    m_valid = true;
}

void DProcessInfo::readProcessCmdline()
{
    const QString cmdlinePath = QStringLiteral("/proc/%1/cmdline").arg(m_pid);
    QFile cmdlineFile(cmdlinePath);
    if (!cmdlineFile.open(QIODevice::ReadOnly))
        return;

    const QByteArray raw = cmdlineFile.readAll();
    if (raw.isEmpty())
        return;

    const QList<QByteArray> parts = raw.split('\0');
    m_arguments.clear();
    for (const QByteArray &part : parts) {
        if (!part.isEmpty())
            m_arguments.append(QString::fromLocal8Bit(part));
    }
}

QString DProcessInfo::readLink(const QString &path) const
{
    char buffer[4096];
    const ssize_t length = ::readlink(path.toLocal8Bit().constData(), buffer, sizeof(buffer) - 1);
    if (length <= 0)
        return QString();
    buffer[length] = '\0';
    return QString::fromLocal8Bit(buffer);
}

void DProcessInfo::readProcessCwd()
{
    m_currentDir = readLink(QStringLiteral("/proc/%1/cwd").arg(m_pid));
}

int DProcessInfo::pid(bool *ok) const
{
    *ok = m_valid;
    return m_pid;
}

int DProcessInfo::parentPid(bool *ok) const
{
    *ok = m_valid && m_parentPid != -1;
    return m_parentPid;
}

int DProcessInfo::foregroundPid(bool *ok) const
{
    *ok = m_valid && m_processGroup != -1;
    return m_processGroup;
}

QString DProcessInfo::name(bool *ok) const
{
    *ok = m_valid;
    return m_name;
}

QStringList DProcessInfo::arguments(bool *ok) const
{
    *ok = m_valid;
    return m_arguments;
}

QString DProcessInfo::currentDir(bool *ok) const
{
    *ok = m_valid && !m_currentDir.isEmpty();
    return m_currentDir;
}

QString DProcessInfo::validCurrentDir() const
{
    if (!m_currentDir.isEmpty())
        return m_currentDir;

    // 目录不可读时上溯父进程
    if (m_parentPid > 0) {
        DProcessInfo parentInfo(m_parentPid);
        parentInfo.update();
        return parentInfo.validCurrentDir();
    }
    return QString();
}

QString DProcessInfo::userName() const
{
    return m_userName;
}

// ---- DShellSession ----

int DShellSession::s_lastSessionId = 0;

DShellSession::DShellSession(QObject *parent)
    : QObject(parent)
    , m_pty(new DTerminalPty(this))
    , m_parser(new DTerminalParser(this))
    , m_monitorTimer(new QTimer(this))
    , m_titleArgsTimer(new QTimer(this))
    , m_sessionId(++s_lastSessionId)
{
    m_sessionProcessInfo = new DProcessInfo(0);
    m_foregroundProcessInfo = new DProcessInfo(0);

    m_pty->setSessionId(m_sessionId);
    m_parser->setSessionId(m_sessionId);

    // 数据通路
    connect(m_pty, &DTerminalPty::dataReceived, this, &DShellSession::onPtyData);
    connect(m_pty, &DTerminalPty::readyReadStandardError, this, []() {});
    connect(m_pty, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DShellSession::onPtyFinished);
    connect(m_parser, &DTerminalParser::dataToSend, m_pty, &DTerminalPty::sendToProcess);
    connect(m_parser, &DTerminalParser::lockPtyRequest, m_pty, &DTerminalPty::lockPty);

    // 标题/外观/状态转发
    connect(m_parser, &DTerminalParser::titleChanged, this, &DShellSession::onParserTitleChanged);
    connect(m_parser, &DTerminalParser::activityChanged, this, &DShellSession::onActivityState);
    connect(m_parser, &DTerminalParser::primaryScreenInUse, this, &DShellSession::onPrimaryScreenInUse);
    connect(m_parser, &DTerminalParser::cursorChanged, this, &DShellSession::cursorChanged);
    connect(m_parser, &DTerminalParser::imageSizeChanged, this, [this](int, int) { updateTerminalSize(); });
    connect(m_parser, &DTerminalParser::imageResizeRequest, this, &DShellSession::resizeRequest);
    connect(m_parser, &DTerminalParser::changeTabTextColorRequest, this, &DShellSession::changeTabTextColorRequest);
    connect(m_parser, &DTerminalParser::profileChangeCommandReceived, this, &DShellSession::profileChangeCommandReceived);
    connect(m_parser, &DTerminalParser::flowControlKeyPressed, this, [this](bool suspend) {
        emit flowControlEnabledChanged(!suspend);
    });
    connect(m_parser, &DTerminalParser::osc52ClipboardRequest, this, &DShellSession::osc52ClipboardRequest);

    // shell 上下文：解析器 → 会话管理器
    m_parser->setForegroundNameProvider([this](int) { return foregroundProcessName(); });

    // 监视
    m_monitorTimer->setSingleShot(true);
    connect(m_monitorTimer, &QTimer::timeout, this, &DShellSession::onMonitorTimer);

    // 标题参数定期更新
    m_titleArgsTimer->setInterval(200);
    connect(m_titleArgsTimer, &QTimer::timeout, this, &DShellSession::onUpdateTitleArgs);
    m_titleArgsTimer->start();

    // 会话管理器登记
    DSessionManager::instance()->addSession(this);
}

DShellSession::~DShellSession()
{
    DSessionManager::instance()->removeSession(this);
    delete m_sessionProcessInfo;
    delete m_foregroundProcessInfo;
    delete m_keymap;
}

void DShellSession::runShell()
{
    QString exec = QString::fromLocal8Bit(QFile::encodeName(m_program));
    QString programPath = exec;

    // 缺省 shell 回退链：$SHELL → /bin/bash
    if (exec.startsWith(QLatin1Char('/')) || exec.isEmpty()) {
        const QString defaultShell{QLatin1String("/bin/bash")};
        QFile excheck(exec);
        if (exec.isEmpty() || !excheck.exists())
            exec = QString::fromLocal8Bit(qgetenv("SHELL"));
        excheck.setFileName(exec);
        if (exec.isEmpty() || !excheck.exists()) {
            qWarning() << "Neither default shell nor $SHELL is set to a correct path. Fallback to" << defaultShell;
            exec = defaultShell;
        }
    }

    if (programPath != exec)
        emit shellWarningMessage(exec, true);

    // 参数可能含空串，join 后过滤
    const QString argsTmp(m_arguments.join(QLatin1Char(' ')).trimmed());
    QStringList arguments;
    arguments << exec;
    if (!argsTmp.isEmpty())
        arguments << m_arguments;

    QStringList environment = m_environment;
    if (!m_initialWorkingDir.isEmpty() && QDir(m_initialWorkingDir).exists()) {
        m_pty->setWorkingDirectory(m_initialWorkingDir);
    }

    // 环境变量补充
    if (m_darkBackground)
        environment << QLatin1String("COLORFGBG=15;0");
    else
        environment << QLatin1String("COLORFGBG=0;15");

    const int result = m_pty->startShell(exec, arguments, environment, false);
    if (result != 0)
        return;

    m_running = true;
    updateTerminalSize();
    emit started();
}

void DShellSession::closeSession()
{
    if (!m_running)
        return;

    m_pty->close();
    if (m_autoClose) {
        m_wantedClose = true;
    } else {
        emit almostFinished();
    }
}

void DShellSession::onPtyFinished(int exitCode, QProcess::ExitStatus status)
{
    Q_UNUSED(exitCode)
    Q_UNUSED(status)

    m_running = false;
    emit finished();
}

void DShellSession::onPtyData(const char *buffer, int length, bool isCommandExec)
{
    m_parser->feedData(buffer, length, isCommandExec);

    if (m_monitorActivity)
        emit activity();

    emit receivedData(QString::fromLatin1(buffer, length));

    // 沉默监视复位
    if (m_monitorSilence && m_notifiedActivity)
        m_notifiedActivity = false;
    if (m_monitorSilence)
        m_monitorTimer->start(m_silenceSeconds * 1000);
}

void DShellSession::onMonitorTimer()
{
    if (m_monitorSilence && !m_notifiedActivity) {
        m_notifiedActivity = true;
        emit silence();
        emit stateChanged(ActivityNormal);
    }
}

void DShellSession::onParserTitleChanged(int what, const QString &value)
{
    setUserTitle(what, value);
}

void DShellSession::setUserTitle(int what, const QString &caption)
{
    bool modified = false;

    if (what == 0 || what == 2) {
        m_isTitleChanged = true;
        if (m_userTitle != caption) {
            m_userTitle = caption;
            modified = true;
        }
    }

    if (what == 0 || what == 1) {
        m_isTitleChanged = true;
        if (m_iconText != caption) {
            m_iconText = caption;
            modified = true;
        }
    }

    if (what == 11) {
        const QString colorString = caption.section(QLatin1Char(';'), 0, 0);
        const QColor backColor(colorString);
        if (backColor.isValid() && backColor != m_modifiedBackground) {
            m_modifiedBackground = backColor;
            emit changeBackgroundColorRequest(backColor);
        }
    }

    if (what == 30) {
        m_isTitleChanged = true;
        if (m_nameTitle != caption) {
            setTitle(NameRole, caption);
            return;
        }
    }

    if (what == 31) {
        QString cwd = caption;
        cwd = cwd.replace(QRegularExpression(QLatin1String("^~")), QDir::homePath());
        emit openUrlRequest(cwd);
    }

    if (what == 32) {
        m_isTitleChanged = true;
        if (m_iconName != caption) {
            m_iconName = caption;
            modified = true;
        }
    }

    if (what == 50) {
        emit profileChangeCommandReceived(caption);
        return;
    }

    if (modified) {
        onUpdateTitleArgs();
        emit titleChanged();
    }
}

void DShellSession::setTitle(TitleRole role, const QString &title)
{
    if (role == NameRole) {
        if (m_nameTitle != title) {
            m_nameTitle = title;
            emit titleChanged();
        }
    } else if (role == DisplayedTitleRole) {
        if (m_displayTitle != title) {
            m_displayTitle = title;
            emit titleChanged();
        }
    }
}

QString DShellSession::title(TitleRole role) const
{
    if (role == NameRole)
        return m_nameTitle;
    return m_displayTitle;
}

// ---- 进程信息 ----

int DShellSession::processId() const
{
    return static_cast<int>(m_pty->processId());
}

void DShellSession::updateSessionProcessInfo()
{
    const int pid = processId();
    if (pid <= 0)
        return;

    delete m_sessionProcessInfo;
    m_sessionProcessInfo = new DProcessInfo(pid);
    m_sessionProcessInfo->update();
}

bool DShellSession::updateForegroundProcessInfo()
{
    const int pgrp = m_pty->foregroundProcessGroup();
    if (pgrp <= 0)
        return false;

    delete m_foregroundProcessInfo;
    m_foregroundProcessInfo = new DProcessInfo(pgrp);
    m_foregroundProcessInfo->update();
    return m_foregroundProcessInfo->isValid();
}

int DShellSession::foregroundProcessId()
{
    bool ok = false;
    if (updateForegroundProcessInfo())
        return m_foregroundProcessInfo->pid(&ok);
    return 0;
}

QString DShellSession::foregroundProcessName()
{
    if (updateForegroundProcessInfo()) {
        bool ok = false;
        const QString name = m_foregroundProcessInfo->name(&ok);
        if (ok)
            return name;
    }
    return QString::fromLocal8Bit(qgetenv("SHELL"));
}

bool DShellSession::isForegroundProcessActive()
{
    updateSessionProcessInfo();
    bool ok = false;
    const int sessionPid = m_sessionProcessInfo->pid(&ok);
    if (!ok)
        return false;
    return foregroundProcessId() != sessionPid;
}

QString DShellSession::workingDirectory()
{
    if (!m_currentWorkingDir.isEmpty())
        return m_currentWorkingDir;
    updateWorkingDirectory();
    return m_currentWorkingDir;
}

void DShellSession::updateWorkingDirectory()
{
    updateSessionProcessInfo();
    const QString currentDir = m_sessionProcessInfo->validCurrentDir();
    if (currentDir != m_currentWorkingDir) {
        m_currentWorkingDir = currentDir;
        emit currentDirectoryChanged(m_currentWorkingDir);
    }
}

void DShellSession::onPrimaryScreenInUse(bool use)
{
    m_primaryScreen = use;
    emit primaryScreenInUse(use);
}

void DShellSession::onActivityState(int state)
{
    emit stateChanged(state);
}

void DShellSession::onUpdateTitleArgs()
{
    updateSessionProcessInfo();

    // 用户名 %u
    const QString userName = m_sessionProcessInfo->userName();
    if (m_userName != userName) {
        m_userName = userName;
        emit titleArgsChange(QLatin1String("%u"), m_userName);
    }

    // 程序名 %n
    bool ok = false;
    const QString programName = m_sessionProcessInfo->name(&ok);
    if (m_programName != programName) {
        m_programName = programName;
        emit titleArgsChange(QLatin1String("%n"), m_programName);
    }
}

// ---- 输入 ----

void DShellSession::sendText(const QString &text) const
{
    m_parser->sendText(text);
}

void DShellSession::sendKeyEvent(QKeyEvent *event) const
{
    m_parser->handleKeyEvent(event);
}

void DShellSession::sendMouseEvent(int buttons, int column, int line, int eventType) const
{
    m_parser->handleMouseEvent(buttons, column, line, eventType);
}

void DShellSession::refresh()
{
    // 让 shell 重绘：窗口尺寸先略增再恢复
    const QSize currentSize = size();
    m_pty->setWindowSize(currentSize.height(), currentSize.width());
    m_pty->setWindowSize(currentSize.height() + 1, currentSize.width());
    m_pty->setWindowSize(currentSize.height(), currentSize.width());
}

// ---- 尺寸 ----

QSize DShellSession::size() const
{
    return m_parser->imageSize();
}

void DShellSession::setSize(const QSize &size)
{
    if (size.width() <= 1 || size.height() <= 1)
        return;

    emit resizeRequest(size);
}

void DShellSession::updateTerminalSize()
{
    const QSize imageSize = m_parser->imageSize();
    m_pty->setWindowSize(imageSize.height(), imageSize.width());
}

// ---- 其他 ----

void DShellSession::setHistoryStorage(DTerminalHistory::StorageKind kind, int maxLines, bool copyContent)
{
    m_parser->setHistoryStorage(kind, maxLines, copyContent);
}

void DShellSession::clearHistory()
{
    m_parser->clearHistory();
}

void DShellSession::setKeymapFile(const QString &path)
{
    delete m_keymap;
    m_keymap = DKeyboardTranslator::loadFromFile(path);
    if (m_keymap != nullptr)
        m_parser->setKeyTranslator(m_keymap);
}

void DShellSession::setCodec(QTextCodec *codec)
{
    m_codec = codec;
    m_parser->setCodec(codec);
    DCharWidth::setGB18030Mode(QString(codec->name()).toUpper().contains(QLatin1String("GB18030")));
}

void DShellSession::setFlowControlEnabled(bool enabled)
{
    m_flowControl = enabled;
    m_pty->setFlowControlEnabled(enabled);
    emit flowControlEnabledChanged(enabled);
}

int DShellSession::ptySlaveFd() const
{
    return m_pty->slaveFd();
}

char DShellSession::getEraseChar()
{
    return m_pty->eraseChar();
}

void DShellSession::setUninstallConfirmer(const std::function<bool(const QString &)> &confirmer)
{
    m_pty->setUninstallConfirmer(confirmer);
}

// ---- DSessionManager ----

DSessionManager *DSessionManager::instance()
{
    static DSessionManager manager;
    return &manager;
}

DSessionManager::DSessionManager(QObject *parent)
    : QObject(parent)
{
    connect(this, &DSessionManager::sessionIdle, this, [this](bool idle) {
        for (DShellSession *session : std::as_const(m_sessions))
            session->pty()->onSessionIdle(idle);
    });
}

void DSessionManager::addSession(DShellSession *session)
{
    m_sessions.insert(session->sessionId(), session);
}

void DSessionManager::removeSession(DShellSession *session)
{
    m_sessions.remove(session->sessionId());
    m_shellPrompts.remove(session->sessionId());
    m_shellCommands.remove(session->sessionId());
}

DShellSession *DSessionManager::sessionById(int sessionId) const
{
    return m_sessions.value(sessionId, nullptr);
}

QList<DShellSession *> DSessionManager::sessions() const
{
    return m_sessions.values();
}

void DSessionManager::saveShellPrompt(int sessionId, const QString &prompt)
{
    m_shellPrompts.insert(sessionId, prompt);
}

QString DSessionManager::currentShellPrompt(int sessionId) const
{
    return m_shellPrompts.value(sessionId);
}

void DSessionManager::saveShellCommand(int sessionId, const QString &command)
{
    m_shellCommands.insert(sessionId, command);

    if (DShellSession *session = m_sessions.value(sessionId, nullptr))
        session->pty()->setCurrentCommand(command);
}

QString DSessionManager::currentShellCommand(int sessionId) const
{
    return m_shellCommands.value(sessionId);
}

void DSessionManager::notifyIdle(bool idle)
{
    emit sessionIdle(idle);
}
