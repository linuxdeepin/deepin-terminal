// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCOMPATTERMINALDRIVER_H
#define DCOMPATTERMINALDRIVER_H

#include "dterminaldriver.h"

class QTermWidget;

/*******************************************************************************
 1. @类名:    DCompatTerminalDriver
 2. @说明:    三方 QTermWidget 的适配驱动。仅在三方模式
            （BUILD_SELF_TERMWIDGET=OFF）编译，把 DTerminalDriver 接口
            原样转发到 QTermWidget 的既有接口，保证默认路径行为不变。
*******************************************************************************/
class DCompatTerminalDriver : public DTerminalDriver
{
public:
    explicit DCompatTerminalDriver(QTermWidget *host);

    QWidget *hostWidget() override;

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

private:
    QTermWidget *m_host = nullptr;
};

#endif  // DCOMPATTERMINALDRIVER_H
