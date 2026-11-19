// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DSHELLSESSION_H
#define DSHELLSESSION_H

#include "dterminalparser.h"

#include <QColor>
#include <QHash>
#include <QList>
#include <QObject>
#include <QProcess>
#include <QSize>
#include <QTimer>

#include <functional>

class QTextCodec;
class DTerminalPty;
class DTerminalView;

/*******************************************************************************
 1. @类名:    DProcessInfo
 2. @说明:    进程信息快照。读取 /proc 获取进程名、
            父进程、命令参数、当前工作目录与前台进程组归属。
*******************************************************************************/
class DProcessInfo
{
public:
    explicit DProcessInfo(int pid);
    ~DProcessInfo();

    void update();
    bool isValid() const { return m_valid; }

    int pid(bool *ok) const;
    int parentPid(bool *ok) const;
    int foregroundPid(bool *ok) const;
    QString name(bool *ok) const;
    QStringList arguments(bool *ok) const;
    QString currentDir(bool *ok) const;
    /** 当前目录；不可读时取父进程目录 */
    QString validCurrentDir() const;
    QString userName() const;
    void setUserNameRequired(bool required) { m_needUserName = required; }

private:
    void readProcessStat();
    void readProcessCmdline();
    void readProcessCwd();
    QString readLink(const QString &path) const;

    int m_pid = 0;
    bool m_valid = false;
    int m_parentPid = -1;
    int m_processGroup = -1;
    int m_session = -1;
    int m_ttyDevice = -1;
    QString m_name;
    QStringList m_arguments;
    QString m_currentDir;
    QString m_userName;
    bool m_needUserName = true;
};

/*******************************************************************************
 2. @类名:    DShellSession
 3. @说明:    终端会话。串起 PTY 与解析器：
            进程输出 → 解析器 → 屏幕；键盘输入 → 解析器 → PTY。
            管理标题/图标、监视、工作目录跟踪、前景进程检测，
            并承载卸载确认回调、标题参数更新。
*******************************************************************************/
class DShellSession : public QObject
{
    Q_OBJECT
public:
    enum TitleRole {
        NameRole,
        DisplayedTitleRole
    };

    explicit DShellSession(QObject *parent = nullptr);
    ~DShellSession() override;

    bool isRunning() const { return m_running; }

    // ---- 配置 ----
    void setShellProgram(const QString &program) { m_program = program; }
    QString shellProgram() const { return m_program; }
    void setShellArguments(const QStringList &arguments) { m_arguments = arguments; }
    void setShellEnvironment(const QStringList &environment) { m_environment = environment; }
    void setInitialWorkingDirectory(const QString &dir) { m_initialWorkingDir = dir; }
    void setHistoryStorage(DTerminalHistory::StorageKind kind, int maxLines, bool copyContent = true);
    void clearHistory();
    /** 从 keytab 文件加载键盘映射 */
    void setKeymapFile(const QString &path);
    void setCodec(QTextCodec *codec);
    void setAutoClose(bool autoClose) { m_autoClose = autoClose; }
    void setFlowControlEnabled(bool enabled);
    bool flowControlEnabled() const { return m_flowControl; }
    void setDarkBackground(bool dark) { m_darkBackground = dark; }

    // ---- 监视 ----
    void setMonitorActivity(bool monitor) { m_monitorActivity = monitor; }
    void setMonitorSilence(bool monitor) { m_monitorSilence = monitor; }
    void setMonitorSilenceSeconds(int seconds) { m_silenceSeconds = seconds; }

    // ---- 标题与图标 ----
    void setTitle(TitleRole role, const QString &title);
    QString title(TitleRole role) const;
    QString nameTitle() const { return title(NameRole); }
    void setUserTitle(int what, const QString &caption);
    QString userTitle() const { return m_userTitle; }
    void setIconName(const QString &iconName) { m_iconName = iconName; }
    QString iconName() const { return m_iconName; }
    void setIconText(const QString &iconText) { m_iconText = iconText; }
    QString iconText() const { return m_iconText; }
    bool isTitleChanged() const { return m_isTitleChanged; }

    // ---- 进程信息 ----
    int processId() const;
    int foregroundProcessId();
    QString foregroundProcessName();
    QString workingDirectory();
    void updateWorkingDirectory();
    void updateSessionProcessInfo();
    bool updateForegroundProcessInfo();
    bool isForegroundProcessActive();
    bool isPrimaryScreen() const { return m_primaryScreen; }

    // ---- 输入 ----
    void sendText(const QString &text) const;
    void sendKeyEvent(QKeyEvent *event) const;
    void sendMouseEvent(int buttons, int column, int line, int eventType) const;
    /** 让 shell 重绘当前显示区域 */
    void refresh();

    // ---- 尺寸 ----
    QSize size() const;
    void setSize(const QSize &size);

    // ---- 其他 ----
    int sessionId() const { return m_sessionId; }
    int ptySlaveFd() const;
    char getEraseChar();

    DTerminalParser *parser() { return m_parser; }
    DTerminalPty *pty() { return m_pty; }
    DProcessInfo *processInfo() { return m_sessionProcessInfo; }

    /** 卸载确认器注入（M6 由门面上层对话框提供） */
    void setUninstallConfirmer(const std::function<bool(const QString &)> &confirmer);

public slots:
    /** 启动终端进程 */
    void runShell();
    /** 关闭会话（SIGHUP + finished） */
    void closeSession();

signals:
    void started();
    void almostFinished();
    void finished();
    void receivedData(const QString &text);
    void titleChanged();
    void profileChanged(const QString &profile);
    void stateChanged(int state);
    void currentDirectoryChanged(const QString &dir);
    void bellRequest(const QString &message);
    void changeTabTextColorRequest(int color);
    void changeBackgroundColorRequest(const QColor &color);
    void openUrlRequest(const QString &url);
    void resizeRequest(const QSize &size);
    void profileChangeCommandReceived(const QString &text);
    void flowControlEnabledChanged(bool enabled);
    void primaryScreenInUse(bool use);
    void cursorChanged(int cursorShape, bool blinkingCursorEnabled);
    void silence();
    void activity();
    void titleArgsChange(QString key, QString value);
    void shellWarningMessage(QString currentShell, bool isSuccess);
    void osc52ClipboardRequest(char target, const QString &base64Data);

private slots:
    void onPtyData(const char *buffer, int length, bool isCommandExec);
    void onPtyFinished(int exitCode, QProcess::ExitStatus status);
    void onMonitorTimer();
    void onParserTitleChanged(int what, const QString &value);
    void onUpdateTitleArgs();
    void onPrimaryScreenInUse(bool use);
    void onActivityState(int state);

private:
    void updateTerminalSize();

    DTerminalPty *m_pty = nullptr;
    DTerminalParser *m_parser = nullptr;
    DKeyboardTranslator *m_keymap = nullptr;
    QTextCodec *m_codec = nullptr;
    DProcessInfo *m_sessionProcessInfo = nullptr;
    DProcessInfo *m_foregroundProcessInfo = nullptr;
    int m_foregroundPid = -1;

    bool m_running = false;
    bool m_monitorActivity = false;
    bool m_monitorSilence = false;
    bool m_notifiedActivity = false;
    bool m_autoClose = false;
    bool m_wantedClose = false;
    bool m_flowControl = true;
    bool m_darkBackground = true;
    QTimer *m_monitorTimer = nullptr;
    int m_silenceSeconds = 10;

    QString m_nameTitle;
    QString m_displayTitle;
    QString m_userTitle;
    QString m_iconName;
    QString m_iconText;
    bool m_isTitleChanged = false;

    QString m_program;
    QStringList m_arguments;
    QStringList m_environment;
    QString m_initialWorkingDir;
    QString m_currentWorkingDir;
    QColor m_modifiedBackground;
    bool m_primaryScreen = true;

    int m_sessionId = 0;
    static int s_lastSessionId;

    // 标题参数
    QString m_userName;
    QString m_programName;
    QString m_currentDirForTitle;
    QTimer *m_titleArgsTimer = nullptr;
};

/*******************************************************************************
 3. @类名:    DSessionManager
 4. @说明:    会话注册表。维护会话集合与
            shell 上下文（提示符/当前命令），供解析器、PTY
            与上层取用；空闲状态统一广播。
*******************************************************************************/
class DSessionManager : public QObject
{
    Q_OBJECT
public:
    static DSessionManager *instance();

    void addSession(DShellSession *session);
    void removeSession(DShellSession *session);
    DShellSession *sessionById(int sessionId) const;
    QList<DShellSession *> sessions() const;

    void saveShellPrompt(int sessionId, const QString &prompt);
    QString currentShellPrompt(int sessionId) const;
    void saveShellCommand(int sessionId, const QString &command);
    QString currentShellCommand(int sessionId) const;

    /** 会话忙闲广播（PTY 卸载检测联动） */
    void notifyIdle(bool idle);

signals:
    void sessionIdle(bool idle);

private:
    explicit DSessionManager(QObject *parent = nullptr);

    QHash<int, DShellSession *> m_sessions;
    QHash<int, QString> m_shellPrompts;
    QHash<int, QString> m_shellCommands;
};

#endif  // DSHELLSESSION_H
