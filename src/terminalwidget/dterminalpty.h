// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTERMINALPTY_H
#define DTERMINALPTY_H

#include <QObject>
#include <QProcess>
#include <QSize>
#include <QStringList>

#include <functional>

class QSocketNotifier;
class QTextCodec;

/*******************************************************************************
 1. @类名:    DTerminalPty
 2. @作者:    ut001213 liuzhangjian
 3. @日期:    2026-11-13
 4. @说明:    终端伪终端（自研直实现）。整合历史实现中
            "进程 + pty 设备 + 事件层" 的多级结构：
            父进程经 posix_openpt 打开主端并监听可读；
            子进程修饰器建立会话、接管控制终端并把从端
            接到标准输入输出。进程退出由 QProcess 机制回收。
            保留 deepin 自有行为：卸载命令检测（remove/purge）、
            expect 密码屏蔽、乱码输出过滤、非 UTF-8 分块缓冲。
*******************************************************************************/
class DTerminalPty : public QProcess
{
    Q_OBJECT
public:
    enum PtyChannel {
        StdinChannel = 1,
        StdoutChannel = 2,
        StderrChannel = 4,
        AllChannels = StdinChannel | StdoutChannel | StderrChannel
    };

    explicit DTerminalPty(QObject *parent = nullptr);
    /** 使用已打开的 pty 主端构造（远程终端直通场景） */
    explicit DTerminalPty(int masterFd, QObject *parent = nullptr);
    ~DTerminalPty() override;

    /** 启动终端进程；返回 0 成功、非 0 失败 */
    int startShell(const QString &program, const QStringList &arguments,
                   const QStringList &environment, bool addToUtmp);

    /** 空 pty 模式（不启动内部进程，仅供外部终端直通） */
    void setEmptyPtyProperties();

    void setWriteable(bool writeable);
    void setFlowControlEnabled(bool on);
    bool flowControlEnabled() const;

    void setWindowSize(int lines, int columns);
    QSize windowSize() const;

    void setEraseChar(char erase);
    char eraseChar() const;

    /** 当前前台进程组（tcgetpgrp），失败返回 0 */
    int foregroundProcessGroup() const;

    /** 挂起/恢复终端进程 */
    void lockPty(bool lock);

    /** 写入当前前台进程组 */
    void sendToProcess(const char *buffer, int length, const QTextCodec *codec);

    int masterFd() const { return m_masterFd; }
    int slaveFd() const { return m_slaveFd; }

    void setSessionId(int sessionId) { m_sessionId = sessionId; }

    /** 设置当前 shell 命令（卸载检测用，由上层会话注入） */
    void setCurrentCommand(const QString &command) { m_currentCommand = command; }

    /** 卸载确认器（同步返回用户是否确认；M6 由上层对话框注入） */
    void setUninstallConfirmer(const std::function<bool(const QString &)> &confirmer) { m_uninstallConfirmer = confirmer; }

    /** 会话空闲联动（确认卸载后等待系统空闲执行 killall；M4 由会话管理器接入） */
    void onSessionIdle(bool isIdle);

signals:
    void dataReceived(const char *buffer, int length, bool isCommandExec);

private slots:
    void onMasterReadable();

private:
    void initPty();
    void handleIncomingData(const QByteArray &data);
    bool isTerminalRemoved();
    static bool matchesCommandPattern(const QString &command, const QString &pattern);
    bool wouldRemoveTerminal(const QString &command);
    bool wouldPurgeTerminal(const QString &command);

    int m_masterFd = -1;
    int m_slaveFd = -1;
    QSocketNotifier *m_readNotifier = nullptr;
    int m_windowColumns = 80;
    int m_windowLines = 24;
    char m_erase = '\b';
    bool m_xonXoff = false;
    bool m_utf8 = true;   // 会话编码：默认 UTF-8，sendToProcess 按会话 codec 同步
    int m_sessionId = 0;
    bool m_uninstallDetected = false;

    // deepin 自有行为状态
    bool m_blockCommandOutput = false;   // expect 密码屏蔽
    int m_blockIndex = -1;
    const QTextCodec *m_textCodec = nullptr;
    bool m_isCommandExec = false;
    QString m_program;
    QString m_currentCommand;
    QByteArray m_pendingBytes;           // 等待换行的非 UTF-8 缓冲
    std::function<bool(const QString &)> m_uninstallConfirmer;
};

#endif  // DTERMINALPTY_H
