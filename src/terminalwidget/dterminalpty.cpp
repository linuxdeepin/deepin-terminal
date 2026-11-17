// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dterminalpty.h"

#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSocketNotifier>
#include <QTextCodec>

#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

// ---- pty 建立 ----

void DTerminalPty::initPty()
{
    // 打开主端
    m_masterFd = ::posix_openpt(O_RDWR | O_NOCTTY);
    if (m_masterFd == -1)
        return;

    if (::grantpt(m_masterFd) != 0 || ::unlockpt(m_masterFd) != 0) {
        ::close(m_masterFd);
        m_masterFd = -1;
        return;
    }

    // 子进程修饰器：会话 + 控制终端 + 标准流接管
    setChildProcessModifier([this]() {
        if (m_masterFd == -1)
            return;

        const char *slaveName = ::ptsname(m_masterFd);
        if (slaveName == nullptr)
            return;

        m_slaveFd = ::open(slaveName, O_RDWR | O_NOCTTY);
        if (m_slaveFd == -1)
            return;

        ::setsid();
        ::ioctl(m_slaveFd, TIOCSCTTY, 1);
        ::dup2(m_slaveFd, 0);
        ::dup2(m_slaveFd, 1);
        ::dup2(m_slaveFd, 2);
        if (m_slaveFd > 2)
            ::close(m_slaveFd);
    });

    m_readNotifier = new QSocketNotifier(m_masterFd, QSocketNotifier::Read, this);
    connect(m_readNotifier, &QSocketNotifier::activated, this, &DTerminalPty::onMasterReadable);
}

DTerminalPty::DTerminalPty(QObject *parent)
    : QProcess(parent)
{
    initPty();
}

DTerminalPty::DTerminalPty(int masterFd, QObject *parent)
    : QProcess(parent)
    , m_masterFd(masterFd)
{
    if (m_masterFd != -1) {
        m_readNotifier = new QSocketNotifier(m_masterFd, QSocketNotifier::Read, this);
        connect(m_readNotifier, &QSocketNotifier::activated, this, &DTerminalPty::onMasterReadable);
    }
}

DTerminalPty::~DTerminalPty()
{
    // 先关 pty 让 shell 收到 SIGHUP
    if (m_masterFd != -1) {
        ::close(m_masterFd);
        m_masterFd = -1;
    }
    waitForFinished(300);
    if (state() != QProcess::NotRunning) {
        qWarning() << "DTerminalPty: process still running, sending SIGHUP";
        ::kill(static_cast<pid_t>(processId()), SIGHUP);
        waitForFinished(300);
        if (state() != QProcess::NotRunning)
            qCritical() << "DTerminalPty: process didn't stop upon SIGHUP";
    }
}

int DTerminalPty::startShell(const QString &program, const QStringList &arguments,
                             const QStringList &environment, bool addToUtmp)
{
    Q_UNUSED(addToUtmp)

    m_program = program;

    // 参数列表首项是程序名本身（历史约定），剥离后交给 QProcess
    Q_ASSERT(arguments.count() >= 1);
    const QStringList userArguments = arguments.mid(1);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    for (const QString &pair : environment) {
        const int separator = pair.indexOf(QLatin1Char('='));
        if (separator > 0)
            env.insert(pair.left(separator), pair.mid(separator + 1));
    }
    setProcessEnvironment(env);

    start(program, userArguments, QIODevice::ReadWrite);
    if (!waitForStarted(3000))
        return 1;

    return 0;
}

void DTerminalPty::setEmptyPtyProperties()
{
    setProcessChannelMode(QProcess::SeparateChannels);
    setInputChannelMode(QProcess::ManagedInputChannel);
}

void DTerminalPty::setWriteable(bool writeable)
{
    if (writeable)
        setInputChannelMode(QProcess::ManagedInputChannel);
    else
        setInputChannelMode(QProcess::ForwardedInputChannel);
}

// ---- 窗口与终端属性 ----

void DTerminalPty::setWindowSize(int lines, int columns)
{
    m_windowLines = lines;
    m_windowColumns = columns;

    if (m_masterFd == -1)
        return;

    winsize size;
    size.ws_row = static_cast<ushort>(lines);
    size.ws_col = static_cast<ushort>(columns);
    size.ws_xpixel = 0;
    size.ws_ypixel = 0;
    ::ioctl(m_masterFd, TIOCSWINSZ, &size);
}

QSize DTerminalPty::windowSize() const
{
    return QSize(m_windowColumns, m_windowLines);
}

void DTerminalPty::setEraseChar(char erase)
{
    m_erase = erase;
}

char DTerminalPty::eraseChar() const
{
    return m_erase;
}

void DTerminalPty::setFlowControlEnabled(bool on)
{
    m_xonXoff = on;
    if (m_masterFd == -1)
        return;

    termios tty;
    ::tcgetattr(m_masterFd, &tty);
    if (on) {
        tty.c_iflag |= IXON | IXOFF;
        tty.c_cc[VSTART] = 17;  // ^Q
        tty.c_cc[VSTOP] = 19;   // ^S
    } else {
        tty.c_iflag &= ~(IXON | IXOFF);
        tty.c_cc[VSTART] = 0;
        tty.c_cc[VSTOP] = 0;
    }
    ::tcsetattr(m_masterFd, TCSANOW, &tty);
}

bool DTerminalPty::flowControlEnabled() const
{
    if (m_masterFd == -1)
        return m_xonXoff;

    termios tty;
    ::tcgetattr(m_masterFd, &tty);
    return (tty.c_iflag & IXON) != 0;
}

int DTerminalPty::foregroundProcessGroup() const
{
    if (m_masterFd == -1)
        return 0;
    const int pgrp = ::tcgetpgrp(m_masterFd);
    if (pgrp == -1)
        return 0;
    return pgrp;
}

void DTerminalPty::lockPty(bool lock)
{
    const pid_t pid = static_cast<pid_t>(processId());
    if (pid <= 0)
        return;

    if (lock)
        ::kill(pid, SIGSTOP);
    else
        ::kill(pid, SIGCONT);
}

// ---- 数据通路 ----

void DTerminalPty::sendToProcess(const char *buffer, int length, const QTextCodec *codec)
{
    m_textCodec = codec;

    // 与 handleIncomingData 的非 UTF-8 行缓冲联动：UTF-8 是默认会话编码，
    // 仅 GB 系编码启用行缓冲。fish 等 shell 的回车重绘块不含换行（⏎ 弃行
    // hack + 提示符），UTF-8 下误入行缓冲会被 m_pendingBytes 永久扣留，
    // 表现为首次回车后提示符不再绘制
    m_utf8 = (codec == nullptr)
            || QString(codec->name()).toUpper().startsWith(QLatin1String("UTF"));

    if (length == 0)
        return;

    // 自定义命令面板触发的命令（以换行结尾）
    bool isCustomCommand = false;
    const QString currentCommand = QString::fromLatin1(buffer);
    if (!currentCommand.isEmpty() && currentCommand.endsWith(QLatin1Char('\n')))
        isCustomCommand = true;

    m_isCommandExec = false;
    m_blockCommandOutput = false;

    // 检测到回车（用户执行命令）
    if ((*buffer == '\r' || isCustomCommand) && !m_uninstallDetected) {
        m_isCommandExec = true;
        QString shellCommand = m_currentCommand;
        if (isCustomCommand)
            shellCommand = currentCommand;

        // 远程管理发送的 expect 密码需要屏蔽回显
        const bool sentByRemoteManage = property("isSendByRemoteManage").toBool();
        if (sentByRemoteManage && shellCommand.startsWith(QLatin1String("expect -f"))) {
            m_blockCommandOutput = true;
            setProperty("isSendByRemoteManage", QVariant(false));
        }

        // 卸载命令检测（deepin 自有功能）：确认后等待系统空闲执行清理
        if (!isTerminalRemoved() && (wouldRemoveTerminal(shellCommand) || wouldPurgeTerminal(shellCommand))) {
            const QString removalKind = wouldPurgeTerminal(shellCommand) ? QLatin1String("purge")
                                                                         : QLatin1String("remove");
            if (m_uninstallConfirmer) {
                const bool confirmed = m_uninstallConfirmer(removalKind);
                if (confirmed) {
                    qDebug() << "确认卸载终端！";
                    m_uninstallDetected = true;
                } else {
                    qDebug() << "不卸载终端！";
                    return;
                }
            }
        }
    }

    // GB 系编码且非命令执行：先转 UTF-8 再写入（deepin 自有编码修复）
    if (m_textCodec != nullptr && QString(m_textCodec->name()).toUpper().startsWith(QLatin1String("GB"))
            && !m_isCommandExec) {
        QTextCodec *utf8Codec = QTextCodec::codecForName("UTF-8");
        const QString unicodeData = m_textCodec->toUnicode(buffer, length);
        const QByteArray utf8Data = utf8Codec->fromUnicode(unicodeData);
        ::write(m_masterFd, utf8Data.constData(), utf8Data.length());
        return;
    }

    ::write(m_masterFd, buffer, length);
}

void DTerminalPty::onSessionIdle(bool isIdle)
{
    if (!m_uninstallDetected)
        return;

    if (isIdle) {
        // 卸载完成，关闭所有终端窗口
        if (isTerminalRemoved())
            ::system("killall deepin-terminal");
    }

    if (!isTerminalRemoved())
        m_uninstallDetected = false;
}

void DTerminalPty::onMasterReadable()
{
    char buffer[4096];
    const ssize_t count = ::read(m_masterFd, buffer, sizeof(buffer));
    if (count <= 0) {
        if (count == 0 || errno != EAGAIN) {
            // 主端关闭
            m_readNotifier->setEnabled(false);
        }
        return;
    }

    handleIncomingData(QByteArray(buffer, static_cast<int>(count)));
}

// ---- deepin 自有行为（移植整合） ----

bool DTerminalPty::isTerminalRemoved()
{
    return !QFile::exists(QLatin1String("/usr/bin/deepin-terminal"));
}

bool DTerminalPty::matchesCommandPattern(const QString &command, const QString &pattern)
{
    QString trimmed = command.trimmed();
    const QRegularExpression expression(pattern);
    const QRegularExpressionValidator validator(expression, nullptr);
    int pos = 0;
    return validator.validate(trimmed, pos) == QValidator::Acceptable;
}

namespace {
// 按 && 与 ; 拆出含包名的子命令
QStringList splitCommandCandidates(const QString &command, const QString &packageName)
{
    QStringList candidates;
    candidates.append(command);

    const QChar separators[] = {QLatin1Char('&'), QLatin1Char(';')};
    for (const QChar separator : separators) {
        const QStringList parts = command.split(separator);
        for (const QString &part : parts) {
            const QString trimmed = part.trimmed();
            if (!trimmed.isEmpty() && trimmed.contains(packageName))
                candidates.append(trimmed);
        }
    }
    return candidates;
}
}  // namespace

bool DTerminalPty::wouldRemoveTerminal(const QString &command)
{
    const QString packageName = QLatin1String("deepin-terminal");
    const QStringList candidates = splitCommandCandidates(command, packageName);

    const QStringList patterns = {
        QStringLiteral("sudo\\s+apt-get\\s+remove\\s+%1").arg(packageName),
        QStringLiteral("sudo\\s+apt\\s+remove\\s+%1").arg(packageName),
        QStringLiteral("sudo\\s+dpkg\\s+-r\\s+%1").arg(packageName),
        QStringLiteral("sudo\\s+rm\\s+.+\\s+/usr/bin/deepin-terminal"),
        QStringLiteral("sudo\\s+rm\\s+/usr/bin/deepin-terminal")
    };

    for (const QString &candidate : candidates) {
        for (const QString &pattern : patterns) {
            if (matchesCommandPattern(candidate, pattern))
                return true;
        }
    }
    return false;
}

bool DTerminalPty::wouldPurgeTerminal(const QString &command)
{
    const QString packageName = QLatin1String("deepin-terminal");
    const QStringList candidates = splitCommandCandidates(command, packageName);

    const QStringList patterns = {
        QStringLiteral("sudo\\s+apt-get\\s+purge\\s+%1").arg(packageName),
        QStringLiteral("sudo\\s+apt-get\\s+purge\\s+-y\\s+%1").arg(packageName),
        QStringLiteral("sudo\\s+apt-get\\s+remove\\s+--purge\\s+%1").arg(packageName),
        QStringLiteral("sudo\\s+apt-get\\s+--purge\\s+remove\\s+%1").arg(packageName),
        QStringLiteral("sudo\\s+apt\\s+purge\\s+%1").arg(packageName),
        QStringLiteral("sudo\\s+apt\\s+purge\\s+-y\\s+%1").arg(packageName),
        QStringLiteral("sudo\\s+apt\\s+remove\\s+--purge\\s+%1").arg(packageName),
        QStringLiteral("sudo\\s+apt\\s+--purge\\s+remove\\s+%1").arg(packageName),
        QStringLiteral("sudo\\s+dpkg\\s+-P\\s+%1").arg(packageName)
    };

    for (const QString &candidate : candidates) {
        for (const QString &pattern : patterns) {
            if (matchesCommandPattern(candidate, pattern))
                return true;
        }
    }
    return false;
}

void DTerminalPty::handleIncomingData(const QByteArray &data)
{
    QByteArray outData = data;
    QString recvData = QString::fromUtf8(outData);

    // expect 密码屏蔽流程
    if (m_blockCommandOutput) {
        QString judgeData = recvData;
        if (recvData.length() > 1) {
            judgeData = judgeData.replace(QLatin1Char('\r'), QString());
            judgeData = judgeData.replace(QLatin1Char('\n'), QString());
        }

        // zsh 会残留一个字母 'e'
        if (m_program.endsWith(QLatin1String("/zsh"))
                && judgeData.length() == 1
                && judgeData.startsWith(QLatin1Char('e'))
                && m_blockIndex == -1) {
            m_blockIndex = 0;
            return;
        }

        // 远程登录敏感信息（expect -f 后的明文密码）
        if (judgeData.startsWith(QLatin1String("expect -f"))
                || judgeData.startsWith(QLatin1String("\bexpect"))
                || judgeData.startsWith(QLatin1String("\be"))
                || judgeData.startsWith(QLatin1String("e\bexpect"))
                || judgeData.startsWith(QLatin1String("e\be"))) {
            m_blockIndex = 1;
            return;
        }

        if (m_blockIndex >= 1) {
            if (judgeData.contains(QLatin1String("Press"))) {
                m_blockCommandOutput = false;
                m_blockIndex = -1;
                const int pressIndex = recvData.indexOf(QLatin1String("Press"));
                if (pressIndex > 0)
                    recvData = recvData.mid(pressIndex);
                const QString helpData = recvData.replace(QLatin1Char('\n'), QString());
                recvData = QLatin1String("\r\n") + helpData + QLatin1String("\r\n");
                outData = recvData.toUtf8();
                emit dataReceived(outData.constData(), outData.count(), m_isCommandExec);
            } else {
                ++m_blockIndex;
            }
            return;
        }
    }

    // 上传下载乱码抑制（deepin bug 修复）
    if (recvData.contains(QLatin1String("bash: $'\\212"))
            || recvData.contains(QStringLiteral("bash: **0800000000022d："))
            || recvData.contains(QStringLiteral("**^XB0800000000022d"))
            || recvData.startsWith(QStringLiteral("**\u0018B0800000000022d\r\u008A"))) {
        return;
    }

    // \u008A 乱码替换，避免退格效果导致命令错乱
    if (m_utf8 && recvData.contains(QStringLiteral("\u008A"))) {
        recvData.replace(QStringLiteral("\u008A"), QStringLiteral("\b \b #"));
        outData = recvData.toUtf8();
    }

    if (recvData == QLatin1String("rz waiting to receive.")) {
        recvData += QLatin1String("\r\n");
        outData = recvData.toUtf8();
    }

    // 非 UTF-8 编码：命令输出按行缓冲，避免截断
    if (!m_utf8) {
        if (m_isCommandExec) {
            const int newlineIndex = outData.lastIndexOf('\n');
            if (newlineIndex == -1) {
                m_pendingBytes.append(outData);
                // 结束符出现则整段送出
                if (m_pendingBytes.contains("\x1B[00m")) {
                    emit dataReceived(m_pendingBytes.constData(), m_pendingBytes.size(), true);
                    m_pendingBytes.clear();
                }
                return;
            } else if (newlineIndex != outData.count() - 1) {
                const int remainingCount = outData.size() - newlineIndex - 1;
                // 行尾剩余字节并入缓冲等待后续换行
                m_pendingBytes.append(outData.left(newlineIndex + 1));
                outData = outData.right(remainingCount);
                emit dataReceived(m_pendingBytes.constData(), m_pendingBytes.size(), true);
                m_pendingBytes.clear();
            }
        } else {
            if (!m_pendingBytes.isEmpty()) {
                m_pendingBytes.append(outData);
                outData = m_pendingBytes;
                m_pendingBytes.clear();
            }
        }
    }

    emit dataReceived(outData.constData(), outData.count(), m_isCommandExec);
}
