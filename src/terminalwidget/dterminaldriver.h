// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTERMINALDRIVER_H
#define DTERMINALDRIVER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QPoint>
#include <QUrl>
#include <QFont>
#include <QWidget>

#include "dterminalcompat.h"

class QAction;
class QTextCodec;

/*******************************************************************************
 1. @类名:    DTerminalDriver
 2. @作者:    ut001213 liuzhangjian
 3. @日期:    2026-11-12
 4. @说明:    终端实现的统一驱动接口。上层 TermWidget 只依赖该接口，
            由 DCompatTerminalDriver（三方 QTermWidget 适配，仅 OFF 模式编译）
            或 DTerminalWidget（自研实现，ON 模式）提供，实现编译期后端切换。
*******************************************************************************/
class DTerminalDriver : public QWidget
{
    Q_OBJECT
public:
    /** 光标形状，取值与配置项（0/1/2）对齐 */
    enum class DCursorStyle {
        Block = 0,
        Underline = 1,
        IBeam = 2
    };

    explicit DTerminalDriver(QWidget *parent = nullptr);
    ~DTerminalDriver() override = default;

    /** 承载终端内容的窗口部件 */
    virtual QWidget *hostWidget() = 0;

    // ---- 会话与进程 ----
    virtual void startShell() = 0;
    virtual int shellSessionId() = 0;
    virtual bool hasLiveProcess() = 0;
    virtual int foregroundProcessId() = 0;
    virtual QString shellTitle() = 0;

    // ---- 外观与行为配置 ----
    virtual void setHistoryLimit(int lines) = 0;
    virtual void setWordChars(const QString &chars) = 0;
    virtual void setShellEnv(const QStringList &environment) = 0;
    virtual void setShellPath(const QString &program) = 0;
    virtual void setShellArgs(const QStringList &arguments) = 0;
    virtual void setWorkDir(const QString &directory) = 0;
    virtual QString workDir() = 0;
    virtual void applyOpacity(qreal level) = 0;
    virtual void applyColorScheme(const QString &name, bool reloadTheme = false) = 0;
    virtual void setCodec(QTextCodec *codec) = 0;
    virtual void setAutoClose(bool autoClose) = 0;
    virtual QFont terminalFont() = 0;
    virtual void applyFont(const QFont &font) = 0;
    virtual void setCursorStyle(DCursorStyle style) = 0;
    virtual void setCursorBlink(bool blink) = 0;
    virtual void enableCursorReposition(bool enable) = 0;
    virtual void setFlowControl(bool enable) = 0;
    virtual bool flowControlOn() = 0;
    virtual void setBidi(bool enable) = 0;
    virtual void setPasteScrolling(int mode) = 0;

    // ---- 输入输出 ----
    virtual void sendToShell(const QString &text) = 0;
    virtual char eraseChar() = 0;
    virtual void setBackspaceSequence(char *key, int length) = 0;
    virtual void setDeleteSequence(char *key, int length) = 0;

    // ---- 滚动 ----
    virtual bool allowScrollOnOutput() = 0;
    virtual void setAllowScrollOnOutput(bool allow) = 0;
    virtual void setFollowOutput(bool enable) = 0;
    virtual bool atOutputEnd() = 0;

    // ---- 选区与剪贴板 ----
    virtual QString selectionText() = 0;
    virtual void copyToClipboard() = 0;
    virtual void pasteFromClipboard() = 0;
    virtual void selectAllText() = 0;
    virtual void resetSelection() = 0;
    virtual void resetSelectionGuard() = 0;

    // ---- 搜索 ----
    virtual void searchText(const QString &keyword, bool forwards, bool next) = 0;

    // ---- 缩放与热点菜单 ----
    virtual void stepZoomIn() = 0;
    virtual void stepZoomOut() = 0;
    virtual QList<QAction *> hotspotActions(const QPoint &position) = 0;

signals:
    void shellFinished();
    void copyStateChanged(bool hasSelection);
    void focusInTerminal();
    void focusLeftTerminal();
    void leftMousePressed();
    void urlTriggered(const QUrl &url, bool fromContextMenu);
    void outputReceived(const QString &text);
    void removeRequested(const QString &commandName);
    void droppedUrlsArrived(const char *urls);
    void shellAlert(const QString &currentShell, bool isSuccess);
    void titleUpdated();
    void titleArgUpdated(const QString &key, const QString &value);
    void idleStateChanged(bool idle);
    void searchHitMissing();
    void searchHitFound();
    void clipboardRequestFromShell(char target, const QString &base64Data);
};

#endif  // DTERMINALDRIVER_H
