// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dterminalwidget.h"

#include <QAction>
#include <QTextCodec>
#include <QDebug>

// TODO(M2-M5): 以下模块落地后逐一填充实现：
//   M2 DTerminalHistory / DTerminalScreen / DTerminalUtil / DTextFilter
//   M3 DTerminalParser / DKeyboardTranslator
//   M4 DTerminalPty / DShellSession / DEncodingDetector
//   M5 DTerminalView / DColorScheme / DHistorySearch
// M1 阶段仅保证接口齐全、可在自研模式下编译链接。

DTerminalWidget::DTerminalWidget(QWidget *parent)
    : DTerminalDriver(parent)
{
}

QWidget *DTerminalWidget::hostWidget()
{
    return this;
}

QStringList DTerminalWidget::availableColorSchemes()
{
    return QStringList();
}

void DTerminalWidget::startShell()
{
    qWarning("DTerminalWidget::startShell is not implemented yet (M1 skeleton)");
}

int DTerminalWidget::shellSessionId()
{
    return 0;
}

bool DTerminalWidget::hasLiveProcess()
{
    return false;
}

int DTerminalWidget::foregroundProcessId()
{
    return -1;
}

QString DTerminalWidget::shellTitle()
{
    return QString();
}

void DTerminalWidget::setHistoryLimit(int lines)
{
    Q_UNUSED(lines)
}

void DTerminalWidget::setWordChars(const QString &chars)
{
    Q_UNUSED(chars)
}

void DTerminalWidget::setShellEnv(const QStringList &environment)
{
    Q_UNUSED(environment)
}

void DTerminalWidget::setShellPath(const QString &program)
{
    Q_UNUSED(program)
}

void DTerminalWidget::setShellArgs(const QStringList &arguments)
{
    Q_UNUSED(arguments)
}

void DTerminalWidget::setWorkDir(const QString &directory)
{
    Q_UNUSED(directory)
}

QString DTerminalWidget::workDir()
{
    return QString();
}

void DTerminalWidget::applyOpacity(qreal level)
{
    Q_UNUSED(level)
}

void DTerminalWidget::applyColorScheme(const QString &name, bool reloadTheme)
{
    Q_UNUSED(name)
    Q_UNUSED(reloadTheme)
}

void DTerminalWidget::setCodec(QTextCodec *codec)
{
    Q_UNUSED(codec)
}

void DTerminalWidget::setAutoClose(bool autoClose)
{
    Q_UNUSED(autoClose)
}

QFont DTerminalWidget::terminalFont()
{
    return QFont();
}

void DTerminalWidget::applyFont(const QFont &font)
{
    Q_UNUSED(font)
}

void DTerminalWidget::setCursorStyle(DCursorStyle style)
{
    Q_UNUSED(style)
}

void DTerminalWidget::setCursorBlink(bool blink)
{
    Q_UNUSED(blink)
}

void DTerminalWidget::enableCursorReposition(bool enable)
{
    Q_UNUSED(enable)
}

void DTerminalWidget::setFlowControl(bool enable)
{
    Q_UNUSED(enable)
}

bool DTerminalWidget::flowControlOn()
{
    return false;
}

void DTerminalWidget::setBidi(bool enable)
{
    Q_UNUSED(enable)
}

void DTerminalWidget::setPasteScrolling(int mode)
{
    Q_UNUSED(mode)
}

void DTerminalWidget::sendToShell(const QString &text)
{
    Q_UNUSED(text)
}

char DTerminalWidget::eraseChar()
{
    return 0;
}

void DTerminalWidget::setBackspaceSequence(char *key, int length)
{
    Q_UNUSED(key)
    Q_UNUSED(length)
}

void DTerminalWidget::setDeleteSequence(char *key, int length)
{
    Q_UNUSED(key)
    Q_UNUSED(length)
}

bool DTerminalWidget::allowScrollOnOutput()
{
    return true;
}

void DTerminalWidget::setAllowScrollOnOutput(bool allow)
{
    Q_UNUSED(allow)
}

void DTerminalWidget::setFollowOutput(bool enable)
{
    Q_UNUSED(enable)
}

bool DTerminalWidget::atOutputEnd()
{
    return true;
}

QString DTerminalWidget::selectionText()
{
    return QString();
}

void DTerminalWidget::copyToClipboard()
{
}

void DTerminalWidget::pasteFromClipboard()
{
}

void DTerminalWidget::selectAllText()
{
}

void DTerminalWidget::resetSelection()
{
}

void DTerminalWidget::resetSelectionGuard()
{
}

void DTerminalWidget::searchText(const QString &keyword, bool forwards, bool next)
{
    Q_UNUSED(keyword)
    Q_UNUSED(forwards)
    Q_UNUSED(next)
}

void DTerminalWidget::stepZoomIn()
{
}

void DTerminalWidget::stepZoomOut()
{
}

QList<QAction *> DTerminalWidget::hotspotActions(const QPoint &position)
{
    Q_UNUSED(position)
    return QList<QAction *>();
}
