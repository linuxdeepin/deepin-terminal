// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dcompatterminaldriver.h"

#include "qtermwidget.h"

DCompatTerminalDriver::DCompatTerminalDriver(QTermWidget *host)
    : DTerminalDriver(host)
    , m_host(host)
{
    // 三方信号 → 驱动信号（逐条透传，保持连接顺序）
    connect(m_host, &QTermWidget::finished, this, &DTerminalDriver::shellFinished);
    connect(m_host, &QTermWidget::copyAvailable, this, &DTerminalDriver::copyStateChanged);
    connect(m_host, &QTermWidget::termGetFocus, this, &DTerminalDriver::focusInTerminal);
    connect(m_host, &QTermWidget::termLostFocus, this, &DTerminalDriver::focusLeftTerminal);
    connect(m_host, &QTermWidget::leftMouseClick, this, &DTerminalDriver::leftMousePressed);
    connect(m_host, &QTermWidget::urlActivated, this, &DTerminalDriver::urlTriggered);
    connect(m_host, &QTermWidget::receivedData, this, &DTerminalDriver::outputReceived);
    connect(m_host, &QTermWidget::uninstallTerminal, this, &DTerminalDriver::removeRequested);
    connect(m_host, &QTermWidget::sendUrlsToTerm, this, &DTerminalDriver::droppedUrlsArrived);
    connect(m_host, &QTermWidget::shellWarningMessage, this, &DTerminalDriver::shellAlert);
    connect(m_host, &QTermWidget::titleChanged, this, &DTerminalDriver::titleUpdated);
    connect(m_host, &QTermWidget::titleArgsChange, this, &DTerminalDriver::titleArgUpdated);
    connect(m_host, &QTermWidget::isTermIdle, this, &DTerminalDriver::idleStateChanged);
    connect(m_host, &QTermWidget::sig_noMatchFound, this, &DTerminalDriver::searchHitMissing);
    connect(m_host, &QTermWidget::sig_matchFound, this, &DTerminalDriver::searchHitFound);
    connect(m_host, &QTermWidget::osc52ClipboardRequest, this, &DTerminalDriver::clipboardRequestFromShell);
}

QWidget *DCompatTerminalDriver::hostWidget()
{
    return m_host;
}

void DCompatTerminalDriver::startShell()
{
    m_host->startShellProgram();
}

int DCompatTerminalDriver::shellSessionId()
{
    return m_host->getSessionId();
}

bool DCompatTerminalDriver::hasLiveProcess()
{
    return m_host->hasRunningProcess();
}

int DCompatTerminalDriver::foregroundProcessId()
{
    return m_host->getForegroundProcessId();
}

QString DCompatTerminalDriver::shellTitle()
{
    return m_host->title();
}

void DCompatTerminalDriver::setHistoryLimit(int lines)
{
    m_host->setHistorySize(lines);
}

void DCompatTerminalDriver::setWordChars(const QString &chars)
{
    m_host->setTerminalWordCharacters(chars);
}

void DCompatTerminalDriver::setShellEnv(const QStringList &environment)
{
    m_host->setEnvironment(environment);
}

void DCompatTerminalDriver::setShellPath(const QString &program)
{
    m_host->setShellProgram(program);
}

void DCompatTerminalDriver::setShellArgs(const QStringList &arguments)
{
    m_host->setArgs(arguments);
}

void DCompatTerminalDriver::setWorkDir(const QString &directory)
{
    m_host->setWorkingDirectory(directory);
}

QString DCompatTerminalDriver::workDir()
{
    return m_host->workingDirectory();
}

void DCompatTerminalDriver::applyOpacity(qreal level)
{
    m_host->setTerminalOpacity(level);
}

void DCompatTerminalDriver::applyColorScheme(const QString &name, bool reloadTheme)
{
    m_host->setColorScheme(name, reloadTheme);
}

void DCompatTerminalDriver::setCodec(QTextCodec *codec)
{
    m_host->setTextCodec(codec);
}

void DCompatTerminalDriver::setAutoClose(bool autoClose)
{
    m_host->setAutoClose(autoClose);
}

QFont DCompatTerminalDriver::terminalFont()
{
    return m_host->getTerminalFont();
}

void DCompatTerminalDriver::applyFont(const QFont &font)
{
    m_host->setTerminalFont(font);
}

void DCompatTerminalDriver::setCursorStyle(DCursorStyle style)
{
    m_host->setKeyboardCursorShape(static_cast<QTermWidget::KeyboardCursorShape>(style));
}

void DCompatTerminalDriver::setCursorBlink(bool blink)
{
    m_host->setBlinkingCursor(blink);
}

void DCompatTerminalDriver::enableCursorReposition(bool enable)
{
    m_host->enableSetCursorPosition(enable);
}

void DCompatTerminalDriver::setFlowControl(bool enable)
{
    m_host->setFlowControlEnabled(enable);
}

bool DCompatTerminalDriver::flowControlOn()
{
    return m_host->flowControlEnabled();
}

void DCompatTerminalDriver::setBidi(bool enable)
{
    m_host->setBidiEnabled(enable);
}

void DCompatTerminalDriver::setPasteScrolling(int mode)
{
    m_host->setMotionAfterPasting(mode);
}

void DCompatTerminalDriver::sendToShell(const QString &text)
{
    m_host->sendText(text);
}

char DCompatTerminalDriver::eraseChar()
{
    return m_host->getErase();
}

void DCompatTerminalDriver::setBackspaceSequence(char *key, int length)
{
    m_host->setBackspaceMode(key, length);
}

void DCompatTerminalDriver::setDeleteSequence(char *key, int length)
{
    m_host->setDeleteMode(key, length);
}

bool DCompatTerminalDriver::allowScrollOnOutput()
{
    return m_host->getIsAllowScroll();
}

void DCompatTerminalDriver::setAllowScrollOnOutput(bool allow)
{
    m_host->setIsAllowScroll(allow);
}

void DCompatTerminalDriver::setFollowOutput(bool enable)
{
    m_host->setTrackOutput(enable);
}

bool DCompatTerminalDriver::atOutputEnd()
{
    return m_host->isAtEndOfOutput();
}

QString DCompatTerminalDriver::selectionText()
{
    return m_host->selectedText();
}

void DCompatTerminalDriver::copyToClipboard()
{
    m_host->copyClipboard();
}

void DCompatTerminalDriver::pasteFromClipboard()
{
    m_host->pasteClipboard();
}

void DCompatTerminalDriver::selectAllText()
{
    m_host->setSelectionAll();
}

void DCompatTerminalDriver::resetSelection()
{
    m_host->clearSelection();
}

void DCompatTerminalDriver::resetSelectionGuard()
{
    m_host->setNoHasSelect();
}

void DCompatTerminalDriver::searchText(const QString &keyword, bool forwards, bool next)
{
    m_host->search(keyword, forwards, next);
}

void DCompatTerminalDriver::stepZoomIn()
{
    m_host->zoomIn();
}

void DCompatTerminalDriver::stepZoomOut()
{
    m_host->zoomOut();
}

QList<QAction *> DCompatTerminalDriver::hotspotActions(const QPoint &position)
{
    return m_host->filterActions(position);
}
