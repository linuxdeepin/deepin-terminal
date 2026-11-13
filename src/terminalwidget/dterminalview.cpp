// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dterminalview.h"

#include "dcolorscheme.h"
#include "dterminalcompat.h"
#include "dterminalparser.h"
#include "dterminalscreen.h"

#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QGestureEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QPanGesture>
#include <QPinchGesture>
#include <QScrollBar>
#include <QSwipeGesture>
#include <QTapAndHoldGesture>
#include <QTapGesture>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QWheelEvent>

#include <cmath>

bool DTerminalView::s_antialiasText = true;

namespace {

constexpr int kTextBlinkDelay = 500;
constexpr int kCursorBlinkDelay = 500;

// 单元格宽度（显示列数）判定：跳过宽字符延续格后取实际字符宽度
int cellDisplayWidth(const DTerminalChar &cell)
{
    if (cell.codePoint == 0)
        return 0;  // 延续格
    const int width = DCharWidth::cellWidth(cell.codePoint);
    return width > 0 ? width : 1;
}

}  // namespace

DTerminalView::DTerminalView(QWidget *parent)
    : QWidget(parent)
    , m_scrollBar(new QScrollBar(this))
    , m_blinkTimer(new QTimer(this))
    , m_blinkCursorTimer(new QTimer(this))
    , m_resizeTimer(new QTimer(this))
    , m_inertiaTimer(new QTimer(this))
{
    // 默认配色（深底白字）
    const DColorScheme defaultScheme;
    defaultScheme.fillPalette(m_palette);

    setAttribute(Qt::WA_InputMethodEnabled, true);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setFocusPolicy(Qt::WheelFocus);
    setMouseTracking(true);

    // 滚动条
    m_scrollBar->setCursor(Qt::ArrowCursor);
    m_scrollBar->setMinimum(0);
    m_scrollBar->setMaximum(0);
    m_scrollBar->setValue(0);
    m_scrollBar->setPageStep(1);
    m_scrollBar->setSingleStep(1);
    connect(m_scrollBar, &QScrollBar::valueChanged, this, [this](int value) {
        scrollTo(value);
    });

    // 闪烁
    m_blinkTimer->setInterval(kTextBlinkDelay);
    connect(m_blinkTimer, &QTimer::timeout, this, &DTerminalView::blinkEvent);
    m_blinkCursorTimer->setInterval(kCursorBlinkDelay);
    connect(m_blinkCursorTimer, &QTimer::timeout, this, &DTerminalView::blinkCursorEvent);

    // resize 提示
    m_resizeTimer->setSingleShot(true);
    m_resizeTimer->setInterval(1000);
    connect(m_resizeTimer, &QTimer::timeout, this, [this]() {
        if (m_resizeWidget) {
            m_resizeWidget->hide();
            m_resizeWidget = nullptr;
        }
    });

    // 惯性滚动
    m_inertiaTimer->setInterval(16);
    connect(m_inertiaTimer, &QTimer::timeout, this, [this]() {
        slideGesture(m_inertiaValue);
    });

    setAcceptDrops(true);
    grabGesture(Qt::PinchGesture);
    grabGesture(Qt::PanGesture);
    grabGesture(Qt::SwipeGesture);
    grabGesture(Qt::TapGesture);
    grabGesture(Qt::TapAndHoldGesture);

    setWindowLineCount(0);
}

DTerminalView::~DTerminalView() = default;

// ---- 屏幕绑定 ----

void DTerminalView::setScreen(DTerminalScreen *screen)
{
    m_screen = screen;
    m_currentLine = 0;
    m_windowLines = 0;
    setWindowLineCount(0);
    updateImageSize();
}

void DTerminalView::setWindowLineCount(int lines)
{
    if (lines == m_windowLines)
        return;
    m_windowLines = lines;
    if (m_screen)
        makeImage();
}

void DTerminalView::updateImageSize()
{
    m_usedLines = m_screen ? m_screen->lineCount() : 0;
    m_usedColumns = m_screen ? m_screen->columnCount() : 0;

    makeImage();
}

void DTerminalView::makeImage()
{
    if (m_screen == nullptr)
        return;

    m_lines = (height() - 1 - m_topMargin * 2) / m_fontHeight;
    if (m_lines <= 0)
        m_lines = 1;
    m_columns = (width() - 1 - m_leftMargin * 2) / m_fontWidth;
    if (m_columns <= 0)
        m_columns = 1;

    m_image.resize(m_lines * m_columns);
    m_image.fill(DTerminalChar());
    m_lineFlags.resize(m_lines);
    m_lineFlags.fill(LineDefault);

    // 同步滚动条范围
    const int totalLines = m_screen->lineCount() + m_screen->historyLineCount();
    const int scrollMax = qMax(0, totalLines - m_lines);
    m_scrollBar->setMaximum(scrollMax);
    m_scrollBar->setPageStep(m_lines);
    m_scrollBar->setValue(qBound(0, m_currentLine, scrollMax));

    updateImage();
}

void DTerminalView::updateImage()
{
    if (m_screen == nullptr)
        return;

    const int historyLines = m_screen->historyLineCount();
    const int mergedStart = m_currentLine;
    const int mergedEnd = qMin(mergedStart + m_lines - 1,
                               historyLines + m_screen->lineCount() - 1);
    if (mergedEnd < mergedStart) {
        m_image.fill(DTerminalChar());
        m_lineFlags.fill(LineDefault);
        update();
        return;
    }

    m_screen->getImage(m_image.data(), m_image.size(), mergedStart, mergedEnd);
    m_lineFlags = m_screen->getLineFlags(mergedStart, mergedEnd);

    // 闪烁字符扫描
    m_hasBlinker = false;
    for (const DTerminalChar &cell : m_image) {
        if (cell.style & StyleBlink) {
            m_hasBlinker = true;
            break;
        }
    }

    if (m_hasBlinker && !m_blinkTimer->isActive())
        m_blinkTimer->start();
    if (!m_hasBlinker && m_blinkTimer->isActive())
        m_blinkTimer->stop();

    update();
}

void DTerminalView::updateLineProperties()
{
    if (m_screen == nullptr)
        return;

    const int historyLines = m_screen->historyLineCount();
    const int mergedStart = m_currentLine;
    const int mergedEnd = qMin(mergedStart + m_lines - 1,
                               historyLines + m_screen->lineCount() - 1);
    if (mergedEnd >= mergedStart)
        m_lineFlags = m_screen->getLineFlags(mergedStart, mergedEnd);

    update();
}

void DTerminalView::notifyOutputChanged()
{
    if (m_screen == nullptr)
        return;

    if (m_trackOutput && m_isAllowScroll) {
        const int historyLines = m_screen->historyLineCount();
        m_currentLine = historyLines;  // 跟随输出
    }

    m_scrollCount += m_screen->scrolledLines();
    m_screen->resetScrolledLines();

    const int totalLines = m_screen->lineCount() + m_screen->historyLineCount();
    const int scrollMax = qMax(0, totalLines - m_lines);
    m_scrollBar->setMaximum(scrollMax);
    m_scrollBar->setValue(qBound(0, m_currentLine, scrollMax));

    updateImage();
    processFilters();

    emit scrolled(m_currentLine);
}

void DTerminalView::scrollTo(int line)
{
    if (m_screen == nullptr)
        return;

    const int totalLines = m_screen->lineCount() + m_screen->historyLineCount();
    const int scrollMax = qMax(0, totalLines - m_lines);
    m_currentLine = qBound(0, line, scrollMax);
    m_scrollBar->setValue(m_currentLine);

    updateImage();
    update();
}

void DTerminalView::scrollBy(RelativeScrollMode mode, int amount)
{
    if (mode == ScrollPages)
        amount *= m_lines;
    scrollTo(m_currentLine + amount);
}

void DTerminalView::scrollToEnd()
{
    if (m_screen == nullptr)
        return;
    const int totalLines = m_screen->lineCount() + m_screen->historyLineCount();
    scrollTo(qMax(0, totalLines - m_lines));
}

bool DTerminalView::atEndOfOutput() const
{
    if (m_screen == nullptr)
        return true;
    const int totalLines = m_screen->lineCount() + m_screen->historyLineCount();
    return m_currentLine + m_lines >= totalLines;
}

QPoint DTerminalView::cursorPosition() const
{
    if (m_screen == nullptr)
        return QPoint(0, 0);
    return QPoint(m_screen->cursorColumn(), m_screen->cursorRow() + m_screen->historyLineCount() - m_currentLine);
}

int DTerminalView::windowColumnCount() const
{
    return m_screen ? m_screen->columnCount() : 0;
}

// ---- 几何 ----

void DTerminalView::calcGeometry()
{
    m_fontHeight = fontMetrics().height() + m_drawTextAdditionHeight;
    m_fontWidth = fontMetrics().horizontalAdvance(QLatin1Char(' '));
    m_fontAscent = fontMetrics().ascent();
    m_fixedFont = fontMetrics().horizontalAdvance(QLatin1Char('i'))
            == fontMetrics().horizontalAdvance(QLatin1Char('m'));

    m_contentRect = QRect(m_leftMargin + m_leftBaseMargin, m_topMargin + m_topBaseMargin,
                          qMax(0, width() - m_contentHeight - 2 * (m_leftMargin + m_leftBaseMargin)),
                          qMax(0, height() - m_contentWidth - 2 * (m_topMargin + m_topBaseMargin)));

    if (m_centerContents)
        m_contentRect.moveCenter(rect().center());

    m_contentHeight = m_fontHeight * m_lines;
    m_contentWidth = m_fontWidth * m_columns;
}

void DTerminalView::propagateSize()
{
    if (m_isFixedSize) {
        setSize(m_columns, m_lines);
        QWidget::setFixedSize(sizeHint());
    }

    if (m_image.size() != m_lines * m_columns)
        makeImage();

    emit fontMetricChanged(m_fontHeight, m_fontWidth);
    emit contentSizeChanged(m_contentHeight, m_contentWidth);

    updateImage();
}

void DTerminalView::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    m_userIsResizing = true;

    // resize 期间显示尺寸提示
    if (m_terminalSizeHint && m_resizeWidget == nullptr) {
        m_resizeWidget = new QLabel(QStringLiteral("%1x%2").arg(m_columns).arg(m_lines), this);
        m_resizeWidget->setAlignment(Qt::AlignCenter);
        m_resizeWidget->setAutoFillBackground(true);
        m_resizeWidget->adjustSize();
        m_resizeWidget->move(width() / 2 - m_resizeWidget->width() / 2,
                             height() / 2 - m_resizeWidget->height() / 2);
        m_resizeWidget->show();
    }

    updateImageSize();
    propagateSize();
    update();

    if (m_resizeTimer->isActive())
        m_resizeTimer->stop();
    m_resizeTimer->start();
    m_userIsResizing = false;
}

QSize DTerminalView::sizeHint() const
{
    return QSize(m_fontWidth * m_columns + 1 + m_leftMargin * 2,
                 m_fontHeight * m_lines + 1 + m_topMargin * 2);
}

void DTerminalView::setSize(int columns, int lines)
{
    if (m_screen == nullptr)
        return;

    m_lines = lines;
    m_columns = columns;

    propagateSize();
    update();
}

void DTerminalView::setFixedSize(int columns, int lines)
{
    m_isFixedSize = true;
    setSize(columns, lines);
}

// ---- 字体 ----

void DTerminalView::setFont(const QFont &font)
{
    QWidget::setFont(font);
    setVTFont(font);
}

void DTerminalView::setVTFont(const QFont &font)
{
    // 字号限制（deepin 自有 BUG42250/BUG42412 修复移植）
    if (font.pointSize() < Konsole::__minFontSize || font.pointSize() > Konsole::__maxFontSize)
        return;

    QFont adjusted = font;
    adjusted.setKerning(false);
    adjusted.setFixedPitch(true);
    QWidget::setFont(adjusted);

    calcGeometry();
    propagateSize();
    update();
}

// ---- 绘制 ----

void DTerminalView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing, s_antialiasText);

    // 背景
    const QColor background = m_colorsInverted
            ? m_palette[1].color : m_palette[0].color;
    const QColor foreground = m_colorsInverted
            ? m_palette[0].color : m_palette[1].color;

    if (m_backgroundImage.isNull()) {
        painter.fillRect(rect(), background);
    } else {
        painter.drawPixmap(rect(), m_backgroundImage);
    }

    painter.setBackground(background);
    painter.setPen(foreground);

    // 以单元格坐标绘制整屏内容
    if (m_columns > 0 && m_lines > 0)
        drawContent(painter, QRect(0, 0, m_columns - 1, m_lines - 1));

    // 输入法预编辑串
    if (!m_preedit.preeditString.isEmpty())
        drawPreeditString(painter, preeditRect());

    // 热点下划线
    paintScannerSpots(painter);

    // resize 提示
    if (m_terminalSizeHint && m_resizeWidget)
        showResizeNotification();
}

void DTerminalView::drawBackground(QPainter &painter, const QRect &rect,
                                   const QColor &color, bool useOpacity)
{
    QColor fillColor = color;
    if (useOpacity)
        fillColor.setAlphaF(m_palette[1].color.alphaF());
    painter.fillRect(rect, fillColor);
}

void DTerminalView::drawContent(QPainter &painter, const QRect &rect)
{
    if (m_image.isEmpty())
        return;

    QVector<uint> charBuffer;
    charBuffer.reserve(m_columns);

    for (int y = rect.y(); y <= rect.bottom(); ++y) {
        if (y >= m_lines)
            break;

        int x = rect.x();
        if (m_image[y * m_columns + x].codePoint == 0 && x != 0)
            x--;  // 回溯宽字符起点

        for (; x <= rect.right(); ++x) {
            int length = 1;
            int position = 0;

            charBuffer.resize(m_columns);
            uint *buffer = charBuffer.data();

            const DTerminalChar &cell = m_image[y * m_columns + x];
            if (cell.style & StyleExtended) {
                ushort extendedLength = 0;
                const uint *chars = DCharSequenceTable::instance()->lookup(cell.codePoint, extendedLength);
                if (chars != nullptr && extendedLength > 1) {
                    charBuffer.resize(m_columns + extendedLength - 1);
                    buffer = charBuffer.data();
                    for (ushort i = 0; i < extendedLength; ++i)
                        buffer[position++] = chars[i];
                }
            } else if (cell.codePoint != 0) {
                buffer[position++] = cell.codePoint;
            }

            // 按相同样式聚合连续片段
            const DTerminalChar &styleRef = cell;
            while (x + length <= rect.right() && x + length < m_columns) {
                const DTerminalChar &next = m_image[y * m_columns + x + length];
                if (next.style != styleRef.style || next.fg != styleRef.fg || next.bg != styleRef.bg)
                    break;
                if (next.codePoint == 0)
                    break;
                if (next.style & StyleExtended)
                    break;
                buffer[position++] = next.codePoint;
                length++;
            }

            const QString text = QString::fromUcs4(charBuffer.constData(), position);
            const QRect textRect = QRect(m_leftMargin + m_leftBaseMargin + x * m_fontWidth,
                                         m_topMargin + m_topBaseMargin + y * m_fontHeight,
                                         m_fontWidth * length, m_fontHeight);
            drawTextFragment(painter, textRect, text, styleRef);

            x += length - 1;
        }
    }
}

void DTerminalView::drawTextFragment(QPainter &painter, const QRect &rect,
                                     const QString &text, const DTerminalChar &style)
{
    painter.save();

    const QColor backgroundColor = style.bg.space == ColorSpaceSystem
            ? m_palette[2 + style.bg.primary + (style.bg.secondary ? 10 : 0)].color
            : style.bg.space == ColorSpaceDefault
              ? m_palette[style.bg.primary == kDefaultForeIndex ? 0 : 1].color
              : QColor(style.bg.primary, style.bg.secondary, style.bg.tertiary);

    // 与显示背景不同才画底
    if (backgroundColor != painter.background().color())
        drawBackground(painter, rect, backgroundColor, false);

    bool invertCharacterColor = false;
    if (!m_hideCursor && (style.style & StyleCursor)) {
        const QColor cursorForeground = style.fg.space == ColorSpaceSystem
                ? m_palette[2 + style.fg.primary + (style.fg.secondary ? 10 : 0)].color
                : m_palette[0].color;
        drawCursor(painter, rect, cursorForeground, backgroundColor, invertCharacterColor);
    }

    drawCharacters(painter, rect, text, style, invertCharacterColor);

    painter.restore();
}

void DTerminalView::drawCharacters(QPainter &painter, const QRect &rect,
                                   const QString &text, const DTerminalChar &style,
                                   bool invertCharacterColor)
{
    // 闪烁期隐藏
    if (m_blinking && (style.style & StyleBlink))
        return;

    // 隐匿字符
    if (style.style & StyleConceal)
        return;

    const bool useBold = ((style.style & StyleBold) && m_boldIntense) || font().bold();
    const bool useUnderline = (style.style & StyleUnderline) || font().underline();
    const bool useItalic = (style.style & StyleItalic) || font().italic();
    const bool useStrikeOut = (style.style & StyleStrikeout) || font().strikeOut();

    QFont paintFont = painter.font();
    if (paintFont.bold() != useBold
            || paintFont.underline() != useUnderline
            || paintFont.italic() != useItalic
            || paintFont.strikeOut() != useStrikeOut) {
        paintFont.setBold(useBold);
        paintFont.setUnderline(useUnderline);
        paintFont.setItalic(useItalic);
        paintFont.setStrikeOut(useStrikeOut);
        painter.setFont(paintFont);
    }

    // 前景色
    QColor color;
    if (style.fg.space == ColorSpaceSystem)
        color = m_palette[2 + style.fg.primary + (style.fg.secondary ? 10 : 0)].color;
    else if (style.fg.space == ColorSpaceDefault)
        color = m_palette[style.fg.primary == kDefaultForeIndex ? 0 : 1].color;
    else if (style.fg.space == ColorSpaceRgb)
        color = QColor(style.fg.primary, style.fg.secondary, style.fg.tertiary);
    else
        color = painter.pen().color();

    if (invertCharacterColor) {
        color = painter.background().color();
    }
    painter.setPen(color);

    // 线绘字符自绘
    if (m_drawLineChars && isLineCharString(text)) {
        drawLineCharString(painter, rect.x(), rect.y(), text, style);
    } else {
        painter.drawText(rect, Qt::AlignBottom | Qt::AlignLeft, text);
    }
}

bool DTerminalView::isLineCharString(const QString &text) const
{
    for (const QChar c : text) {
        const uint codePoint = c.unicode();
        if (!(0x2500 <= codePoint && codePoint <= 0x257F))
            return false;
    }
    return true;
}

void DTerminalView::drawLineCharString(QPainter &painter, int x, int y,
                                       const QString &text, const DTerminalChar &style)
{
    Q_UNUSED(style)
    int offset = 0;
    for (const QChar c : text) {
        drawLineChar(painter, x + offset * m_fontWidth, y, m_fontWidth, m_fontHeight,
                     c.unicode(), false);
        offset++;
    }
}

// 盒线字符过程化绘制（覆盖常用 16 种；其余回退字体渲染）
void DTerminalView::drawLineChar(QPainter &painter, int x, int y, int w, int h,
                                 uint codePoint, bool bold)
{
    Q_UNUSED(bold)
    Q_UNUSED(codePoint)
    // 简化实现：以字体渲染线绘字符，保持视觉一致的基础路径
    painter.drawText(QRect(x, y, w, h), Qt::AlignBottom | Qt::AlignLeft,
                     QString(QChar(codePoint)));
}

void DTerminalView::drawCursor(QPainter &painter, const QRect &rect,
                               const QColor &foregroundColor,
                               const QColor &backgroundColor, bool &invertCharacterColor)
{
    Q_UNUSED(backgroundColor)
    QRectF cursorRect = rect;
    cursorRect.setHeight(m_fontHeight);

    if (m_cursorBlinking)
        return;

    if (m_cursorColor.isValid())
        painter.setPen(m_cursorColor);
    else
        painter.setPen(foregroundColor);

    const qreal halfWidth = 0.5;

    switch (m_cursorShape) {
    case DCursorShape::Block:
        if (hasFocus()) {
            painter.fillRect(cursorRect, m_cursorColor.isValid() ? m_cursorColor : foregroundColor);
            if (!m_cursorColor.isValid())
                invertCharacterColor = true;
        } else {
            painter.drawRect(cursorRect.adjusted(halfWidth, halfWidth, -halfWidth, -halfWidth));
        }
        break;
    case DCursorShape::Underline:
        if (hasFocus())
            painter.fillRect(QRectF(cursorRect.left(), cursorRect.bottom() - 1,
                                    cursorRect.width(), 1),
                             m_cursorColor.isValid() ? m_cursorColor : foregroundColor);
        else
            painter.drawRect(QRectF(cursorRect.left() + halfWidth, cursorRect.bottom() - 2,
                                    cursorRect.width() - 1, 1));
        break;
    case DCursorShape::IBeam:
        if (hasFocus())
            painter.fillRect(QRectF(cursorRect.left(), cursorRect.top(),
                                    1, cursorRect.height()),
                             m_cursorColor.isValid() ? m_cursorColor : foregroundColor);
        else
            painter.drawRect(QRectF(cursorRect.left() + halfWidth, cursorRect.top(),
                                    0, cursorRect.height()));
        break;
    case DCursorShape::BoldUnderline:
        if (hasFocus())
            painter.fillRect(QRectF(cursorRect.left(), cursorRect.bottom() - 3,
                                    cursorRect.width(), 3),
                             m_cursorColor.isValid() ? m_cursorColor : foregroundColor);
        else
            painter.drawRect(QRectF(cursorRect.left() + halfWidth, cursorRect.bottom() - 4,
                                    cursorRect.width() - 1, 3));
        break;
    }
}

void DTerminalView::drawPreeditString(QPainter &painter, const QRect &rect)
{
    painter.setPen(QPen(painter.pen().color(), 1, Qt::DashLine));
    painter.drawText(rect, Qt::AlignBottom | Qt::AlignLeft, m_preedit.preeditString);
}

QRect DTerminalView::preeditRect() const
{
    const QPoint cursor = cursorPosition();
    return QRect(m_leftMargin + m_leftBaseMargin + cursor.x() * m_fontWidth,
                 m_topMargin + m_topBaseMargin + cursor.y() * m_fontHeight,
                 m_fontWidth * 2, m_fontHeight);
}

QRect DTerminalView::imageToWidget(const QRect &imageArea) const
{
    return QRect(m_leftMargin + m_leftBaseMargin + imageArea.x() * m_fontWidth,
                 m_topMargin + m_topBaseMargin + imageArea.y() * m_fontHeight,
                 imageArea.width() * m_fontWidth, imageArea.height() * m_fontHeight);
}

void DTerminalView::paintScannerSpots(QPainter &painter)
{
    if (m_scanner == nullptr)
        return;

    const QList<DTextFilter::Spot *> spots = m_scanner->spots();
    for (DTextFilter::Spot *spot : spots) {
        if (spot->kind() != DTextFilter::Spot::KindLink)
            continue;
        const QRect imageArea(spot->startColumn(), spot->startLine(),
                              spot->endColumn() - spot->startColumn() + 1,
                              spot->endLine() - spot->startLine() + 1);
        const QRect area = imageToWidget(imageArea);
        painter.setPen(QPen(painter.pen().color(), 1, Qt::DashLine));
        painter.drawLine(area.bottomLeft(), area.bottomRight());
    }
}

// ---- 过滤 ----

void DTerminalView::processFilters()
{
    if (m_scanner == nullptr || m_screen == nullptr)
        return;

    const int historyLines = m_screen->historyLineCount();
    const int mergedStart = m_currentLine;
    const int mergedEnd = qMin(mergedStart + m_lines - 1,
                               historyLines + m_screen->lineCount() - 1);
    if (mergedEnd < mergedStart)
        return;

    m_screen->getImage(m_image.data(), m_image.size(), mergedStart, mergedEnd);
    const QVector<quint8> flags = m_screen->getLineFlags(mergedStart, mergedEnd);
    m_scanner->scanImage(m_image.constData(), m_lines, m_columns, flags);
}

QList<QAction *> DTerminalView::filterActions(const QPoint &position)
{
    if (m_scanner == nullptr)
        return QList<QAction *>();

    processFilters();

    const QPoint imagePos(position.x() / m_fontWidth, position.y() / m_fontHeight);
    DTextFilter::Spot *spot = m_scanner->spotAt(imagePos.y(), imagePos.x());
    if (spot != nullptr)
        return spot->actions();
    return QList<QAction *>();
}

// ---- 选区与剪贴板 ----

void DTerminalView::selectionChanged()
{
    if (m_screen == nullptr)
        return;

    const QString selected = m_screen->selectedText(DTerminalScreen::TextKeepBreaks);
    if (!selected.isEmpty()) {
        QApplication::clipboard()->setText(selected, QClipboard::Selection);
        emit copyAvailable(true);
    }
}

void DTerminalView::selectionCleared()
{
    emit copyAvailable(false);
}

void DTerminalView::copyClipboard()
{
    if (m_screen == nullptr)
        return;

    const QString text = m_screen->selectedText(DTerminalScreen::TextKeepBreaks);
    if (!text.isEmpty())
        QApplication::clipboard()->setText(text);
}

void DTerminalView::bracketText(QString &text)
{
    if (m_bracketedPasteMode) {
        text.prepend(QLatin1String("\033[200~"));
        text.append(QLatin1String("\033[201~"));
    }
}

void DTerminalView::pasteClipboard()
{
    QString text = QApplication::clipboard()->text(QClipboard::Clipboard);
    if (text.isEmpty())
        return;

    bracketText(text);
    const QByteArray utf8 = text.toUtf8();
    emit sendStringToEmulation(utf8.constData());
    if (m_motionAfterPasting == MoveEndScreenWindow)
        scrollToEnd();
    else if (m_motionAfterPasting == MoveStartScreenWindow)
        scrollTo(0);
}

void DTerminalView::pasteSelection()
{
    QString text = QApplication::clipboard()->text(QClipboard::Selection);
    if (text.isEmpty())
        return;

    bracketText(text);
    const QByteArray utf8 = text.toUtf8();
    emit sendStringToEmulation(utf8.constData());
}

void DTerminalView::setSelection(const QString &text)
{
    if (text != QApplication::clipboard()->text(QClipboard::Clipboard))
        selectionChanged();
    QApplication::clipboard()->setText(text, QClipboard::Selection);
}

void DTerminalView::setSelectionAll()
{
    if (m_screen == nullptr)
        return;

    m_screen->setSelectionAll();
    selectionChanged();
    update();
}

void DTerminalView::getCharacterPosition(const QPoint &widgetPoint, int &line, int &column) const
{
    column = (widgetPoint.x() - m_leftMargin - m_leftBaseMargin) / m_fontWidth;
    line = (widgetPoint.y() - m_topMargin - m_topBaseMargin) / m_fontHeight;
}

void DTerminalView::initSelectionState()
{
    m_selectionState = 1;  // 左键按下，尚未选择
    m_selectionInitial = m_selectionCurrent;
    m_wordSelectionMode = false;
    m_lineSelectionMode = false;
    m_columnSelectionMode = false;
    m_tripleSelectionBegin = QPoint();
}

void DTerminalView::initKeyboardSelection()
{
    // 键盘 Shift+PageUp/Down 选择：以光标为锚点
    const QPoint cursor = cursorPosition();
    m_selectionInitial = cursor;
    m_selectionCurrent = cursor;
    m_selectionState = 1;
}

QChar DTerminalView::classifyChar(QChar c) const
{
    if (c.isSpace())
        return QLatin1Char(' ');
    if (c.isLetterOrNumber() || m_wordCharacters.contains(c))
        return QLatin1Char('a');
    return c;
}

void DTerminalView::extendSelection(const QPoint &pos)
{
    if (m_screen == nullptr)
        return;

    m_selectionCurrent = pos;
    m_selectionState = 2;

    if (m_wordSelectionMode) {
        // 词选：按词边界扩展
        int line = pos.y();
        int column = pos.x();
        const int historyLines = m_screen->historyLineCount();

        // 屏幕行数据直接以图像访问
        Q_UNUSED(historyLines)
        while (column > 0 && classifyChar(QChar(m_image[line * m_columns + column - 1].codePoint))
               == classifyChar(QChar(m_image[line * m_columns + column].codePoint)))
            column--;
        m_selectionInitial = QPoint(column, line);
        m_selectionCurrent = pos;
    }

    // 屏幕坐标（窗口）→ 全局坐标（含历史）
    const int windowTopLine = m_currentLine;
    m_screen->setSelectionStart(m_selectionInitial.x(), m_selectionInitial.y() + windowTopLine,
                                m_columnSelectionMode);
    m_screen->setSelectionEnd(m_selectionCurrent.x(), m_selectionCurrent.y() + windowTopLine);

    // deepin：选区状态跟踪
    int startLine = 0, startColumn = 0, endLine = 0, endColumn = 0;
    m_screen->getSelectionStart(startColumn, startLine);
    m_screen->getSelectionEnd(endColumn, endLine);
    m_selStartLine = startLine;
    m_selStartColumn = startColumn;
    m_selEndLine = endLine;
    m_selEndColumn = endColumn;
    m_selBegin = true;

    selectionChanged();
    update();
}

// ---- 鼠标 ----

void DTerminalView::mousePressEvent(QMouseEvent *event)
{
    if (m_screen == nullptr)
        return;

    if (event->button() == Qt::LeftButton) {
        setFocus();
        emit leftMouseClicked();
        emit busySelecting(true);

        int charLine = 0, charColumn = 0;
        getCharacterPosition(event->pos(), charLine, charColumn);

        // deepin：Ctrl+点击定位光标
        if (m_cursorPositionSetEnable && (event->modifiers() & Qt::ControlModifier)) {
            int line = charLine;
            int column = charColumn;
            const QPoint cursor = cursorPosition();
            line -= cursor.y();
            column -= cursor.x();
            const int count = line * m_columns + column;
            emit cursorPositionChanged(count);
        }

        const bool selected = m_screen->isSelected(charColumn, charLine + m_currentLine);

        if ((!m_ctrlDrag || event->modifiers() & Qt::ControlModifier) && selected) {
            // 点击已选文本：进入拖拽预备
            m_dragPending = true;
            m_dragStart = event->pos();
        } else {
            m_dragPending = false;

            m_preserveLineBreaks = !((event->modifiers() & Qt::ControlModifier)
                                     && !(event->modifiers() & Qt::AltModifier));
            m_columnSelectionMode = (event->modifiers() & Qt::AltModifier)
                    && (event->modifiers() & Qt::ControlModifier);

            if (m_usesMouse && !(event->modifiers() & Qt::ShiftModifier)) {
                // 程序接管鼠标：报告事件
                emit mouseEvent(0, charColumn + 1, charLine + 1, 0);
                return;
            }

            m_screen->clearSelection();
            const QPoint pos(charColumn, charLine);
            m_selectionInitial = pos;
            m_selectionCurrent = pos;
            initSelectionState();
        }
    } else if (event->button() == Qt::MiddleButton) {
        if (m_usesMouse) {
            int charLine = 0, charColumn = 0;
            getCharacterPosition(event->pos(), charLine, charColumn);
            emit mouseEvent(1, charColumn + 1, charLine + 1, 0);
        } else {
            pasteSelection();
        }
    } else if (event->button() == Qt::RightButton) {
        if (m_usesMouse && !(event->modifiers() & Qt::ShiftModifier)) {
            int charLine = 0, charColumn = 0;
            getCharacterPosition(event->pos(), charLine, charColumn);
            emit mouseEvent(2, charColumn + 1, charLine + 1, 0);
        } else {
            emit contextMenuRequested(event->pos());
        }
    }
}

void DTerminalView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_screen == nullptr)
        return;

    int charLine = 0, charColumn = 0;
    getCharacterPosition(event->pos(), charLine, charColumn);

    // 程序接管鼠标时的移动上报
    if (m_usesMouse && !(event->buttons() & Qt::LeftButton)) {
        emit mouseEvent(3, charColumn + 1, charLine + 1, 1);
        return;
    }

    if (m_dragPending && (event->buttons() & Qt::LeftButton)
            && (event->pos() - m_dragStart).manhattanLength() > QApplication::startDragDistance()) {
        doDrag();
        m_dragPending = false;
        return;
    }

    if (m_selectionState >= 1 && (event->buttons() & Qt::LeftButton)) {
        if (m_selectionState == 1) {
            // 判断词选/行选
            if (m_possibleTripleClick)
                m_lineSelectionMode = true;
        }
        extendSelection(QPoint(charColumn, charLine));
        return;
    }

    // 热点悬停
    if (m_scanner != nullptr) {
        DTextFilter::Spot *spot = m_scanner->spotAt(charLine, charColumn);
        const bool linkHover = spot != nullptr && spot->kind() == DTextFilter::Spot::KindLink;
        setCursor(linkHover ? Qt::PointingHandCursor : Qt::IBeamCursor);
    }
}

void DTerminalView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_screen == nullptr)
        return;

    if (event->button() == Qt::LeftButton) {
        emit busySelecting(false);

        int charLine = 0, charColumn = 0;
        getCharacterPosition(event->pos(), charLine, charColumn);

        if (m_usesMouse) {
            emit mouseEvent(0, charColumn + 1, charLine + 1, 2);
        }

        if (m_selectionState >= 2)
            selectionChanged();

        m_selectionState = 0;
        m_wordSelectionMode = false;
        m_lineSelectionMode = false;
    }
}

void DTerminalView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    int charLine = 0, charColumn = 0;
    getCharacterPosition(event->pos(), charLine, charColumn);

    // 三击窗口
    m_possibleTripleClick = true;
    QTimer::singleShot(QApplication::doubleClickInterval(), this, [this]() {
        m_possibleTripleClick = false;
    });

    m_wordSelectionMode = true;
    extendSelection(QPoint(charColumn, charLine));
}

void DTerminalView::mouseTripleClickEvent(QMouseEvent *event)
{
    int charLine = 0, charColumn = 0;
    getCharacterPosition(event->pos(), charLine, charColumn);

    if (m_tripleClickMode == SelectWholeLine) {
        m_lineSelectionMode = true;
        m_selectionInitial = QPoint(0, charLine);
        extendSelection(QPoint(m_columns - 1, charLine));
    } else {
        m_lineSelectionMode = true;
        m_selectionInitial = QPoint(charColumn, charLine);
        extendSelection(QPoint(m_columns - 1, charLine));
    }
}

void DTerminalView::wheelEvent(QWheelEvent *event)
{
    if (m_screen == nullptr)
        return;

    if (m_usesMouse && m_alternateScrolling) {
        // 备用屏：滚轮转为上下键
        const int delta = event->angleDelta().y();
        if (delta > 0) {
            for (int i = 0; i < 3; ++i) {
                QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
                emit keyPressed(&keyEvent);
            }
        } else if (delta < 0) {
            for (int i = 0; i < 3; ++i) {
                QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
                emit keyPressed(&keyEvent);
            }
        }
        event->accept();
        return;
    }

    const int steps = event->angleDelta().y() / 120;
    scrollBy(ScrollLines, -steps);
    event->accept();
}

// ---- 拖放 ----

void DTerminalView::doDrag()
{
    if (m_screen == nullptr)
        return;

    const QString text = m_screen->selectedText(DTerminalScreen::TextKeepBreaks);
    if (text.isEmpty())
        return;

    auto *mimeData = new QMimeData();
    mimeData->setText(text);

    auto *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::CopyAction);
}

void DTerminalView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasText() || event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void DTerminalView::dropEvent(QDropEvent *event)
{
    QString text;
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        for (const QUrl &url : urls)
            text += url.toLocalFile() + QLatin1Char(' ');
    } else if (event->mimeData()->hasText()) {
        text = event->mimeData()->text();
    }

    if (text.isEmpty())
        return;

    bracketText(text);
    const QByteArray utf8 = text.toUtf8();
    emit sendStringToEmulation(utf8.constData());
    event->acceptProposedAction();
}

// ---- 键盘 ----

void DTerminalView::keyPressEvent(QKeyEvent *event)
{
    bool emitKeyPressSignal = true;

    // 键盘滚动选择（Shift+PageUp/Down）
    if (event->modifiers() == Qt::ShiftModifier) {
        const int maxColumnIndex = m_columns - 1;
        if (event->key() == Qt::Key_PageUp) {
            if (m_scrollBar->value() != m_scrollBar->maximum())
                initKeyboardSelection();
            scrollBy(ScrollPages, -1);
            emitKeyPressSignal = false;
        } else if (event->key() == Qt::Key_PageDown) {
            if (m_scrollBar->value() != m_scrollBar->maximum())
                initKeyboardSelection();
            scrollBy(ScrollPages, 1);
            emitKeyPressSignal = false;
        } else if (event->key() == Qt::Key_Up) {
            extendSelection(QPoint(m_selectionCurrent.x(), qMax(0, m_selectionCurrent.y() - 1)));
            emitKeyPressSignal = false;
        } else if (event->key() == Qt::Key_Down) {
            extendSelection(QPoint(m_selectionCurrent.x(), m_selectionCurrent.y() + 1));
            emitKeyPressSignal = false;
        } else if (event->key() == Qt::Key_Left) {
            extendSelection(QPoint(qMax(0, m_selectionCurrent.x() - 1), m_selectionCurrent.y()));
            emitKeyPressSignal = false;
        } else if (event->key() == Qt::Key_Right) {
            extendSelection(QPoint(qMin(maxColumnIndex, m_selectionCurrent.x() + 1),
                                   m_selectionCurrent.y()));
            emitKeyPressSignal = false;
        }
    }

    if (emitKeyPressSignal)
        emit keyPressed(event);

    event->accept();
}

bool DTerminalView::handleShortcutOverride(QKeyEvent *event)
{
    bool override = false;
    emit shortcutOverrideCheck(event, override);
    return override;
}

// ---- IME ----

void DTerminalView::inputMethodEvent(QInputMethodEvent *event)
{
    QKeyEvent keyEvent(QEvent::KeyPress, 0, Qt::NoModifier, event->commitString());
    emit keyPressed(&keyEvent);

    m_preedit.preeditString = event->preeditString();
    update(preeditRect().united(m_preedit.previousPreeditRect));
    m_preedit.previousPreeditRect = preeditRect();

    event->accept();
}

QVariant DTerminalView::inputMethodQuery(Qt::InputMethodQuery query) const
{
    const QPoint cursorPos = cursorPosition();
    switch (query) {
    case Qt::ImCursorRectangle:
        return imageToWidget(QRect(cursorPos.x(), cursorPos.y(), 1, 1));
    case Qt::ImFont:
        return font();
    case Qt::ImCursorPosition:
        return cursorPos.x();
    case Qt::ImSurroundingText:
        return QString();
    default:
        return QVariant();
    }
}

// ---- 闪烁/响铃 ----

void DTerminalView::blinkEvent()
{
    if (!m_hasBlinker)
        return;

    m_blinking = !m_blinking;
    update();
}

void DTerminalView::blinkCursorEvent()
{
    if (!m_hasBlinkingCursor)
        return;

    m_cursorBlinking = !m_cursorBlinking;
    update();
}

void DTerminalView::setBlinkingCursor(bool blink)
{
    m_hasBlinkingCursor = blink;
    if (blink) {
        if (!m_blinkCursorTimer->isActive())
            m_blinkCursorTimer->start();
    } else {
        m_blinkCursorTimer->stop();
        m_cursorBlinking = false;
    }
    update();
}

void DTerminalView::bell(const QString &message)
{
    switch (m_bellMode) {
    case SystemBeepBell:
        QApplication::beep();
        break;
    case NotifyBell:
        emit bellRequested(message);
        break;
    case VisualBell:
        m_colorsInverted = !m_colorsInverted;
        update();
        QTimer::singleShot(200, this, &DTerminalView::enableBell);
        break;
    case NoBell:
    default:
        break;
    }
}

void DTerminalView::enableBell()
{
    if (m_colorsInverted) {
        m_colorsInverted = false;
        update();
    }
}

void DTerminalView::swapColorTable()
{
    m_colorsInverted = !m_colorsInverted;
    update();
}

// ---- 滚动条与杂项 ----

void DTerminalView::setScrollBarPosition(ScrollBarPosition position)
{
    m_scrollbarLocation = position;
    update();
}

void DTerminalView::setScroll(int cursor, int lines)
{
    Q_UNUSED(lines)
    scrollTo(cursor);
}

void DTerminalView::setUsesMouse(bool uses)
{
    m_usesMouse = uses;
    emit mouseUsageChanged();
}

void DTerminalView::setOpacity(qreal opacity)
{
    Q_UNUSED(opacity)
    // 透明度经调色板背景 alpha 生效
    update();
}

void DTerminalView::setBackgroundImage(const QString &imagePath)
{
    if (imagePath.isEmpty()) {
        m_backgroundImage = QPixmap();
    } else {
        m_backgroundImage = QPixmap(imagePath);
        if (!m_backgroundImage.isNull())
            m_backgroundImage = m_backgroundImage.scaled(size(), Qt::IgnoreAspectRatio,
                                                         Qt::SmoothTransformation);
    }
    update();
}

void DTerminalView::setPalette(const DTerminalPaletteEntry *table)
{
    for (int i = 0; i < 20; ++i)
        m_palette[i] = table[i];
    update();
}

void DTerminalView::setBackgroundColor(const QColor &color)
{
    m_palette[1].color = color;
    m_palette[1].transparent = false;
    update();
}

void DTerminalView::setForegroundColor(const QColor &color)
{
    m_palette[0].color = color;
    update();
}

void DTerminalView::setCursorColor(bool useForegroundColor, const QColor &color)
{
    if (useForegroundColor)
        m_cursorColor = QColor();
    else
        m_cursorColor = color;
    update();
}

void DTerminalView::setLineSpacing(uint spacing)
{
    m_lineSpacing = spacing;
    calcGeometry();
    propagateSize();
    update();
}

void DTerminalView::setMargin(int margin)
{
    m_margin = margin;
    calcGeometry();
    propagateSize();
    update();
}

void DTerminalView::outputSuspended(bool suspended)
{
    if (!m_flowControlWarningEnabled)
        return;

    if (m_outputSuspendedLabel == nullptr) {
        m_outputSuspendedLabel = new QLabel(this);
        m_outputSuspendedLabel->setText(tr("Output has been suspended by pressing Ctrl+S. Press Ctrl+Q to resume."));
        m_outputSuspendedLabel->setAutoFillBackground(true);
        m_outputSuspendedLabel->adjustSize();
    }

    m_outputSuspendedLabel->setVisible(suspended);
    if (suspended) {
        m_outputSuspendedLabel->move(width() / 2 - m_outputSuspendedLabel->width() / 2,
                                     height() / 2 - m_outputSuspendedLabel->height() / 2);
    }
}

void DTerminalView::showResizeNotification()
{
    if (m_resizeWidget == nullptr)
        return;
    m_resizeWidget->setText(QStringLiteral("%1x%2").arg(m_columns).arg(m_lines));
    m_resizeWidget->adjustSize();
    m_resizeWidget->move(width() / 2 - m_resizeWidget->width() / 2,
                         height() / 2 - m_resizeWidget->height() / 2);
    m_resizeWidget->show();
}

void DTerminalView::scrollImage(int lines, const QRect &region)
{
    Q_UNUSED(region)
    scrollBy(ScrollLines, lines);
}

QPoint DTerminalView::cursorCell() const
{
    return cursorPosition();
}

void DTerminalView::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    QWidget::timerEvent(event);
}

bool DTerminalView::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture)
        gestureEvent(static_cast<QGestureEvent *>(event));

    if (event->type() == QEvent::ShortcutOverride)
        handleShortcutOverride(static_cast<QKeyEvent *>(event));

    return QWidget::event(event);
}

void DTerminalView::focusInEvent(QFocusEvent *event)
{
    emit focusInTerminal();
    QWidget::focusInEvent(event);
}

void DTerminalView::focusOutEvent(QFocusEvent *event)
{
    emit focusLeftTerminal();
    QWidget::focusOutEvent(event);
}

void DTerminalView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

void DTerminalView::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

// ---- 手势（deepin 自有移植） ----

void DTerminalView::gestureEvent(QGestureEvent *event)
{
    if (QGesture *gesture = event->gesture(Qt::TapGesture))
        handleTapGesture(static_cast<QTapGesture *>(gesture));
    if (QGesture *gesture = event->gesture(Qt::TapAndHoldGesture))
        handleTapAndHoldGesture(static_cast<QTapAndHoldGesture *>(gesture));
    if (QGesture *gesture = event->gesture(Qt::PanGesture))
        handlePanGesture(static_cast<QPanGesture *>(gesture));
    if (QGesture *gesture = event->gesture(Qt::PinchGesture))
        handlePinchGesture(static_cast<QPinchGesture *>(gesture));
    if (QGesture *gesture = event->gesture(Qt::SwipeGesture))
        handleSwipeGesture(static_cast<QSwipeGesture *>(gesture));
}

void DTerminalView::handleTapGesture(QTapGesture *gesture)
{
    if (gesture->state() == Qt::GestureFinished) {
        m_gestureAction = GestureNone;
        // 单击即点击
        int charLine = 0, charColumn = 0;
        getCharacterPosition(gesture->position().toPoint(), charLine, charColumn);
        QMouseEvent pressEvent(QEvent::MouseButtonPress, gesture->position(),
                               Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        mousePressEvent(&pressEvent);
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, gesture->position(),
                                 Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        mouseReleaseEvent(&releaseEvent);
    }
}

void DTerminalView::handleTapAndHoldGesture(QTapAndHoldGesture *gesture)
{
    if (gesture->state() == Qt::GestureFinished)
        m_gestureAction = GestureNone;
}

void DTerminalView::handlePanGesture(QPanGesture *gesture)
{
    Q_UNUSED(gesture)
}

void DTerminalView::handlePinchGesture(QPinchGesture *gesture)
{
    if (gesture->state() == Qt::GestureStarted) {
        m_currentStepScaleFactor = m_scaleFactor;
    } else if (gesture->state() == Qt::GestureUpdated) {
        const qreal stepScaleFactor = gesture->totalScaleFactor();
        if (qAbs(stepScaleFactor - m_currentStepScaleFactor) >= 0.12) {
            if (stepScaleFactor > m_currentStepScaleFactor) {
                QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Plus,
                                   Qt::ControlModifier | Qt::KeypadModifier);
                emit keyPressed(&keyEvent);
            } else {
                QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Minus,
                                   Qt::ControlModifier | Qt::KeypadModifier);
                emit keyPressed(&keyEvent);
            }
            m_currentStepScaleFactor = stepScaleFactor;
        }
    } else if (gesture->state() == Qt::GestureFinished) {
        m_scaleFactor = gesture->totalScaleFactor();
        m_gestureAction = GestureNone;
    }
}

void DTerminalView::handleSwipeGesture(QSwipeGesture *gesture)
{
    Q_UNUSED(gesture)
}

void DTerminalView::slideGesture(qreal diff)
{
    if (qAbs(diff) < 1.0)
        return;

    scrollBy(ScrollLines, -static_cast<int>(diff / 10));
}
