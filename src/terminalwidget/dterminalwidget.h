// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTERMINALWIDGET_H
#define DTERMINALWIDGET_H

#include "dterminaldriver.h"

/*******************************************************************************
 1. @类名:    DTerminalWidget
 2. @作者:    ut001213 liuzhangjian
 3. @日期:    2026-11-12
 4. @说明:    自研终端组件门面（M1 骨架）。对外接口按功能拆分为
            DTerminalView/DTerminalScreen/DTerminalParser 等模块，
            后续里程碑逐步填充；本阶段仅保证编译期后端切换可用，
            实现体均为占位桩。
*******************************************************************************/
class DTerminalWidget : public DTerminalDriver
{
    Q_OBJECT
public:
    explicit DTerminalWidget(QWidget *parent = nullptr);

    QWidget *hostWidget() override;

    /** 供上层（主窗口）获取配色清单；M5 由 DColorScheme 提供数据 */
    static QStringList availableColorSchemes();

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
};

#endif  // DTERMINALWIDGET_H
