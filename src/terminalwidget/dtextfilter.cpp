// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dtextfilter.h"

#include "dterminalscreen.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QUrl>

#include <algorithm>

// ---- 热点 ----

DTextFilter::Spot::Spot(int startLine, int startColumn, int endLine, int endColumn)
    : m_startLine(startLine)
    , m_startColumn(startColumn)
    , m_endLine(endLine)
    , m_endColumn(endColumn)
{
}

DTextFilter::Spot::~Spot() = default;

QList<QAction *> DTextFilter::Spot::actions()
{
    return QList<QAction *>();
}

// ---- 基础过滤器 ----

DTextFilter::~DTextFilter()
{
    reset();
}

void DTextFilter::reset()
{
    qDeleteAll(m_spots);
    m_spots.clear();
    m_spotsByLine.clear();
}

void DTextFilter::setBuffer(const QString *buffer, const QVector<int> *lineStarts)
{
    m_buffer = buffer;
    m_lineStarts = lineStarts;
}

void DTextFilter::addSpot(Spot *spot)
{
    m_spots.append(spot);
    for (int line = spot->startLine(); line <= spot->endLine(); ++line)
        m_spotsByLine.insert(line, spot);
}

QList<DTextFilter::Spot *> DTextFilter::spots() const
{
    return m_spots;
}

QList<DTextFilter::Spot *> DTextFilter::spotsAtLine(int line) const
{
    return m_spotsByLine.values(line);
}

DTextFilter::Spot *DTextFilter::spotAt(int line, int column) const
{
    const QList<Spot *> candidates = m_spotsByLine.values(line);
    for (Spot *spot : candidates) {
        if (spot->startLine() == line && spot->startColumn() > column)
            continue;
        if (spot->endLine() == line && spot->endColumn() < column)
            continue;
        return spot;
    }
    return nullptr;
}

void DTextFilter::locateInBuffer(int position, int &line, int &column) const
{
    Q_ASSERT(m_lineStarts);
    Q_ASSERT(m_buffer);

    for (int i = 0; i < m_lineStarts->count(); ++i) {
        int nextLine = 0;
        if (i == m_lineStarts->count() - 1)
            nextLine = m_buffer->length() + 1;
        else
            nextLine = m_lineStarts->value(i + 1);

        if (m_lineStarts->value(i) <= position && position < nextLine) {
            line = i;
            column = DTerminalUtil::displayWidth(buffer()->mid(m_lineStarts->value(i), position - m_lineStarts->value(i)));
            return;
        }
    }
}

// ---- 正则过滤器 ----

DRegexFilter::Spot::Spot(int startLine, int startColumn, int endLine, int endColumn)
    : DTextFilter::Spot(startLine, startColumn, endLine, endColumn)
{
    setKind(KindMarker);
}

void DRegexFilter::Spot::activate(const QString &)
{
}

void DRegexFilter::process()
{
    const QString *text = buffer();
    Q_ASSERT(text);

    // 能匹配空串的正则直接放弃，避免死循环
    static const QString emptyString;
    if (m_pattern.pattern().isEmpty() || m_pattern.match(emptyString).hasMatch())
        return;

    QRegularExpressionMatchIterator it = m_pattern.globalMatch(*text);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const int pos = match.capturedStart();
        const int matchedLength = match.capturedLength();

        // 零长匹配跳过，避免死循环
        if (matchedLength == 0)
            continue;

        int startLine = 0;
        int endLine = 0;
        int startColumn = 0;
        int endColumn = 0;

        locateInBuffer(pos, startLine, startColumn);
        locateInBuffer(pos + matchedLength, endLine, endColumn);

        Spot *spot = createSpot(startLine, startColumn, endLine, endColumn);

        QStringList capturedTexts;
        for (int i = 0; i <= match.lastCapturedIndex(); ++i)
            capturedTexts << match.captured(i);
        spot->setCapturedTexts(capturedTexts);

        addSpot(spot);
    }
}

DRegexFilter::Spot *DRegexFilter::createSpot(int startLine, int startColumn, int endLine, int endColumn)
{
    return new Spot(startLine, startColumn, endLine, endColumn);
}

// ---- URL 过滤器 ----

namespace {
// URL 模式（含端口边界，deepin 自有修正移植）：
// 协议:// 或 www. 开头；端口部分受 0-65535 与非贪婪边界约束，
// 避免把边界字符吞进 URL 导致 QUrl::StrictMode 拒绝
const QRegularExpression kFullUrlPattern(QLatin1String("(www\\.(?!\\.)|[a-z][a-z0-9+.-]*://)[\\w.@-]+"
        "([:]((6553[0-5])|(655[0-2][0-9])|(65[0-4][0-9]{2})|(6[0-4][0-9]{3})|([1-5][0-9]{4})|([1-9][0-9]{3})|([1-9][0-9]{2})|([1-9][0-9])|([0-9]))(?![0-9]))?"
        "([/][\\w.@?^=%&/~+#-]+)?"));

// 邮件地址：词字符/点/连字符 @ 词字符/点/连字符 . 词字符
const QRegularExpression kEmailPattern(QLatin1String("\\b(\\w|\\.|-)+@(\\w|\\.|-)+\\.\\w+\\b"));

// 完整 URL 或邮件地址
const QRegularExpression kCompletePattern(QStringLiteral("(") + kFullUrlPattern.pattern()
                                          + QStringLiteral("|") + kEmailPattern.pattern() + QStringLiteral(")"));
}  // namespace

DUrlFilter::DUrlFilter()
{
    setPattern(kCompletePattern);
}

DRegexFilter::Spot *DUrlFilter::createSpot(int startLine, int startColumn, int endLine, int endColumn)
{
    DUrlSpot *spot = new DUrlSpot(startLine, startColumn, endLine, endColumn);
    connect(spot, &DUrlSpot::activated, this, &DUrlFilter::activated);
    return spot;
}

DUrlSpot::DUrlSpot(int startLine, int startColumn, int endLine, int endColumn)
    : DRegexFilter::Spot(startLine, startColumn, endLine, endColumn)
{
    setKind(KindLink);
}

DUrlSpot::~DUrlSpot() = default;

DUrlSpot::UrlKind DUrlSpot::urlKind() const
{
    const QString url = capturedTexts().constFirst();

    if (kFullUrlPattern.match(url).hasMatch())
        return UrlKind::Standard;
    if (kEmailPattern.match(url).hasMatch())
        return UrlKind::Email;
    return UrlKind::Unknown;
}

void DUrlSpot::activate(const QString &actionName)
{
    QString url = capturedTexts().constFirst();
    const UrlKind kind = urlKind();

    if (actionName == QLatin1String("copy-action")) {
        QApplication::clipboard()->setText(url);
        return;
    }

    if (actionName.isEmpty() || actionName == QLatin1String("open-action") || actionName == QLatin1String("click-action")) {
        if (kind == UrlKind::Standard) {
            // 缺协议时补全 http://
            if (!url.contains(QLatin1String("://")))
                url.prepend(QLatin1String("http://"));
        } else if (kind == UrlKind::Email) {
            url.prepend(QLatin1String("mailto:"));
        }

        emit activated(QUrl(url, QUrl::StrictMode), actionName != QLatin1String("click-action"));
    }
}

QList<QAction *> DUrlSpot::actions()
{
    QList<QAction *> list;
    const UrlKind kind = urlKind();

    QAction *openAction = new QAction(this);
    QAction *copyAction = new QAction(this);

    Q_ASSERT(kind == UrlKind::Standard || kind == UrlKind::Email);

    if (kind == UrlKind::Standard) {
        openAction->setText(QObject::tr("Open link"));
        copyAction->setText(QObject::tr("Copy link"));
    } else if (kind == UrlKind::Email) {
        openAction->setText(QObject::tr("Send email to..."));
        copyAction->setText(QObject::tr("Copy email address"));
    }

    // 动作名回传给 activate() 以区分打开/复制
    openAction->setObjectName(QLatin1String("open-action"));
    copyAction->setObjectName(QLatin1String("copy-action"));

    connect(openAction, &QAction::triggered, this, [this, openAction] { activate(openAction->objectName()); });
    connect(copyAction, &QAction::triggered, this, [this, copyAction] { activate(copyAction->objectName()); });

    list << openAction;
    list << copyAction;
    return list;
}

// ---- 过滤器集合 ----

DTextFilterChain::~DTextFilterChain()
{
    qDeleteAll(m_filters);
    m_filters.clear();
}

void DTextFilterChain::removeFilter(DTextFilter *filter)
{
    m_filters.removeAll(filter);
}

void DTextFilterChain::clear()
{
    m_filters.clear();
}

void DTextFilterChain::reset()
{
    for (DTextFilter *filter : m_filters)
        filter->reset();
}

void DTextFilterChain::process()
{
    for (DTextFilter *filter : m_filters)
        filter->process();
}

void DTextFilterChain::setBuffer(const QString *buffer, const QVector<int> *lineStarts)
{
    for (DTextFilter *filter : m_filters)
        filter->setBuffer(buffer, lineStarts);
}

DTextFilter::Spot *DTextFilterChain::spotAt(int line, int column) const
{
    for (DTextFilter *filter : m_filters) {
        DTextFilter::Spot *spot = filter->spotAt(line, column);
        if (spot != nullptr)
            return spot;
    }
    return nullptr;
}

QList<DTextFilter::Spot *> DTextFilterChain::spots() const
{
    QList<DTextFilter::Spot *> list;
    for (DTextFilter *filter : m_filters)
        list << filter->spots();
    return list;
}

// ---- 终端图像扫描器 ----

DTerminalTextScanner::DTerminalTextScanner() = default;

DTerminalTextScanner::~DTerminalTextScanner()
{
    // 扫描器拥有上下文存取器
    delete m_sink;
}

void DTerminalTextScanner::addFilter(DTextFilter *filter)
{
    m_chain.addFilter(filter);
}

QString DTerminalTextScanner::buildLineText(const DTerminalChar *lineCells, int columns) const
{
    QString lineText;
    lineText.reserve(columns);
    for (int col = 0; col < columns; ++col) {
        const DTerminalChar &cell = lineCells[col];
        if (cell.style & StyleExtended) {
            if (m_sequenceResolver)
                lineText += m_sequenceResolver(cell.codePoint);
            else
                lineText += QChar(cell.codePoint);
        } else {
            lineText += QChar(cell.codePoint);
        }
    }

    // 去除行尾普通空白（与历史实现"不含尾部空白"的文本展开一致）
    int end = lineText.length();
    while (end > 0 && lineText.at(end - 1) == QLatin1Char(' ')
           && (lineCells[end - 1].style == StylePlain)) {
        --end;
    }
    lineText.truncate(end);
    return lineText;
}

void DTerminalTextScanner::scanImage(const DTerminalChar *image, int lines, int columns,
                                     const QVector<quint8> &lineFlags)
{
    // 重建共享缓冲
    m_chain.reset();
    m_buffer.clear();
    m_lineStarts.clear();

    QString lastLine;
    for (int i = 0; i < lines; ++i) {
        m_lineStarts.append(m_buffer.length());

        const QString lineText = buildLineText(image + i * columns, columns);
        m_buffer += lineText;

        // 软换行行不加换行符，避免跨行误连
        if (!(lineFlags.value(i, LineDefault) & LineWrapped))
            m_buffer += QLatin1Char('\n');

        const QString trimmed = lineText.trimmed();
        if (!trimmed.isEmpty())
            lastLine = trimmed;
    }

    // 提取 shell 提示符：末行以 $#%> 之一结尾且全文仅出现一次该结尾符
    if (!lastLine.isEmpty()) {
        const QString promptEnds = QStringLiteral("$#%>");
        for (const QChar promptEnd : promptEnds) {
            bool hasOther = false;
            for (const QChar other : promptEnds) {
                if (other == promptEnd)
                    continue;
                if (lastLine.contains(other)) {
                    hasOther = true;
                    break;
                }
            }
            if (!hasOther && lastLine.count(promptEnd) == 1 && lastLine.endsWith(promptEnd)) {
                if (m_sink)
                    m_sink->saveShellPrompt(m_sessionId, lastLine);
                break;
            }
        }
    }

    // 提取当前正在输入的命令
    if (m_sink) {
        const QString shellPrompt = m_sink->currentShellPrompt(m_sessionId);
        const QString currentBuffer = m_buffer.trimmed();
        if (!currentBuffer.isEmpty() && !shellPrompt.isEmpty()) {
            QString command = currentBuffer.split(shellPrompt).last();
            if (shellPrompt.endsWith(QLatin1Char('#')) && !command.contains(QStringLiteral("sudo ")))
                command = QStringLiteral("sudo %1").arg(command);
            m_sink->saveShellCommand(m_sessionId, command);
        }
    }

    // 跑过滤链
    m_chain.setBuffer(&m_buffer, &m_lineStarts);
    m_chain.process();
}

DTextFilter::Spot *DTerminalTextScanner::spotAt(int line, int column) const
{
    return m_chain.spotAt(line, column);
}

QList<DTextFilter::Spot *> DTerminalTextScanner::spots() const
{
    return m_chain.spots();
}

// ---- OSC8 超链接提取 ----

void DUrlExtractor::appendUrlText(QChar c)
{
    if (!reading())
        return;

    if (m_current.text.isEmpty()) {
        // 起始坐标为最后一个已打印字符的位置
        m_current.beginRow = m_screen->cursorRow() + m_screen->historyLineCount();
        m_current.beginColumn = m_screen->cursorColumn() - 1;
    }
    m_current.text += c;
}

void DUrlExtractor::setUrl(const QString &url)
{
    if (m_allowedSchemas.contains(QUrl(url).scheme() + QLatin1String("://"))) {
        m_current.url = url;
    } else {
        abortUrlInput();
    }
}

void DUrlExtractor::abortUrlInput()
{
    m_reading = false;
    m_current = ExtractedUrl();
    m_ignoreNextToggle = true;
}

void DUrlExtractor::endUrlInput()
{
    Q_ASSERT(reading());
    m_reading = false;

    m_current.endRow = m_screen->cursorRow() + m_screen->historyLineCount();
    m_current.endColumn = m_screen->cursorColumn();
    m_history.append(m_current);

    m_current = ExtractedUrl();
}

void DUrlExtractor::toggleUrlInput()
{
    if (m_ignoreNextToggle) {
        m_ignoreNextToggle = false;
        return;
    }

    if (m_reading)
        endUrlInput();
    else
        beginUrlInput();
}

void DUrlExtractor::historyLinesRemoved(int lines)
{
    for (ExtractedUrl &url : m_history) {
        url.beginRow -= lines;
        url.endRow -= lines;
    }
    m_history.erase(std::remove_if(m_history.begin(), m_history.end(),
                                   [](const ExtractedUrl &url) { return url.beginRow < 0; }),
                    m_history.end());
}
