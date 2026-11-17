// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTERMINALWIDGET_H
#define DTERMINALWIDGET_H

#include "dterminaldriver.h"

#include <QWidget>

#include <functional>

class DShellSession;
class DTerminalView;
class DTerminalTextScanner;
class DUrlFilter;

/*******************************************************************************
 1. @类名:    DTerminalWidget
 2. @说明:    终端组件门面。组装会话/解析/屏幕/显示/配色/
            搜索各模块，向上层（TermWidget）提供统一驱动接口。
*******************************************************************************/
class DTerminalWidget : public DTerminalDriver
{
    Q_OBJECT
public:
    explicit DTerminalWidget(QWidget *parent = nullptr);
    ~DTerminalWidget() override;

    QWidget *hostWidget() override;

    /** 供上层（主窗口）获取配色清单 */
    static QStringList availableColorSchemes();

    /** 卸载确认处理器（M6 由 TermWidget 注入上层对话框回调） */
    void setUninstallHandler(const std::function<bool(const QString &)> &handler);

    void startShell() override;
    int shellSessionId() override;
    bool hasLiveProcess() override;
    int foregroundProcessId() override;
    QString shellTitle() override;

    void setHistoryLimit(int lines) override;
    void setWordChars(const QString &chars) override;
    void setShellEnv(const QStringList &environment) override;
    void setShellPath(const QString &program) override;
    void setShellArgs(const QStringList &arguments) override;
    void setWorkDir(const QString &directory) override;
    QString workDir() override;
    void applyOpacity(qreal level) override;
    void applyColorScheme(const QString &name, bool reloadTheme) override;
    void setCodec(QTextCodec *codec) override;
    void setAutoClose(bool autoClose) override;
    QFont terminalFont() override;
    void applyFont(const QFont &font) override;
    void setCursorStyle(DCursorStyle style) override;
    void setCursorBlink(bool blink) override;
    void enableCursorReposition(bool enable) override;
    void setFlowControl(bool enable) override;
    bool flowControlOn() override;
    void setBidi(bool enable) override;
    void setPasteScrolling(int mode) override;

    void sendToShell(const QString &text) override;
    char eraseChar() override;
    void setBackspaceSequence(char *key, int length) override;
    void setDeleteSequence(char *key, int length) override;

    bool allowScrollOnOutput() override;
    void setAllowScrollOnOutput(bool allow) override;
    void setFollowOutput(bool enable) override;
    bool atOutputEnd() override;

    QString selectionText() override;
    void copyToClipboard() override;
    void pasteFromClipboard() override;
    void selectAllText() override;
    void resetSelection() override;
    void resetSelectionGuard() override;

    void searchText(const QString &keyword, bool forwards, bool next) override;

    void stepZoomIn() override;
    void stepZoomOut() override;
    QList<QAction *> hotspotActions(const QPoint &position) override;

private slots:
    void onMatchFound(int startColumn, int startLine, int endColumn, int endLine,
                      int lastBackwardsPosition, int loseChinese, int matchChinese);
    void onNoMatchFound();

private:
    void wireSignals();
    bool confirmUninstall(const QString &kind);

    DShellSession *m_session = nullptr;
    DTerminalView *m_view = nullptr;
    DTerminalTextScanner *m_scanner = nullptr;
    DUrlFilter *m_urlFilter = nullptr;

    // 搜索状态
    bool m_hasSelection = false;
    int m_searchStartColumn = 0;
    int m_searchStartLine = 0;
    int m_searchEndColumn = 0;
    int m_searchEndLine = 0;
    int m_lastBackwardsPosition = -1;
    bool m_isLastForwards = false;

    std::function<bool(const QString &)> m_uninstallHandler;
};

#endif  // DTERMINALWIDGET_H
