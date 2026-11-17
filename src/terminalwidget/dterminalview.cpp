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
#include <QStyle>
#include <QDateTime>
#include <QFile>
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

// 单元格宽度（显示列数）判定：跳过宽字符延续格后取实际字符宽度
int cellDisplayWidth(const DTerminalChar &cell)
{
    if (cell.codePoint == 0)
        return 0;  // 延续格
    const int width = DCharWidth::cellWidth(cell.codePoint);
    return width > 0 ? width : 1;
}

// BiDi run 分块：Common/Inherited 视为可混排
bool isSameScript(QChar::Script script, QChar::Script lastScript)
{
    return (script == lastScript)
        || ((script == QChar::Script_Common) && (lastScript != QChar::Script_Inherited))
        || ((script == QChar::Script_Inherited) && (lastScript != QChar::Script_Common));
}

// 256 色索引 → RGB（xterm 标准映射：0-15 调色板、16-231 色立方、232-255 灰度）
QColor mapIndex256(int index, const DTerminalPaletteEntry (&palette)[20])
{
    if (index < 16)
        return palette[index + 2].color;
    if (index < 232) {
        const int i = index - 16;
        const int r = (i / 36) % 6;
        const int g = (i / 6) % 6;
        const int b = i % 6;
        return QColor(r == 0 ? 0 : r * 40 + 55,
                      g == 0 ? 0 : g * 40 + 55,
                      b == 0 ? 0 : b * 40 + 55);
    }
    const int gray = (index - 232) * 10 + 8;
    return QColor(gray, gray, gray);
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
    // 背景自填充：脏区先擦成终端底色。否则 diff 判"无变化"的行与
    // QWidget::scroll 新暴露的条带保持未初始化的黑色（黑边根因）
    QPalette selfPal = QWidget::palette();
    selfPal.setBrush(QPalette::Window, m_palette[1].color);
    QWidget::setPalette(selfPal);
    setAutoFillBackground(true);
    setFocusPolicy(Qt::WheelFocus);
    setMouseTracking(true);

    // 初始隐藏滚动条，输出超屏后自动显示并保持常显。
    // 滚动条几何不放布局：在 calcGeometry 里手动 resize+move，
    // 放进空布局会被垂直居中（跑到窗口中间）
    m_scrollBar->hide();
    m_scrollbarLocation = NoScrollBar;

    m_scrollBar->setCursor(Qt::ArrowCursor);
    // 15px 宽、底部留 15px 适配窗口圆角
    m_scrollBar->setStyleSheet(QStringLiteral("margin: 0px 0 15px 0;width: 15"));
    // 终端背景对比度不足时滚动条自填充背景（非 transient 样式）
    if (!m_scrollBar->style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, m_scrollBar))
        m_scrollBar->setAutoFillBackground(true);
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
    // 跟随系统光标闪烁间隔（cursorFlashTime()/2）
    m_blinkCursorTimer->setInterval(QApplication::cursorFlashTime() / 2);
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
        // 指数衰减缓动
        slideGesture(m_inertiaVelocity);
        m_inertiaVelocity *= 0.85;
        if (qAbs(m_inertiaVelocity) < 0.5) {
            m_inertiaTimer->stop();
            m_inertiaVelocity = 0;
        }
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

    const QRect area = contentsRect();

    // 滚动条几何：手动定位到窗口侧边并铺满高度
    m_scrollBar->resize(m_scrollBar->sizeHint().width(), area.height());
    const int scrollBarWidth = m_scrollBar->isHidden()
            ? 0 : m_scrollBar->style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, m_scrollBar)
              ? 0 : m_scrollBar->width();
    switch (m_scrollbarLocation) {
    case ScrollBarLeft:
        m_scrollBar->move(area.topLeft());
        break;
    case ScrollBarRight:
    default:
        m_scrollBar->move(area.topRight() - QPoint(m_scrollBar->width() - 1, 0));
        break;
    }

    // 以内容区计算行列数（内容高 = height - 2*topMargin + 1，
    // 内容宽 = width - 2*leftMargin - scrollBar，均不含 -1 偏差）
    m_lines = (area.height() + 1 - m_topMargin * 2) / m_fontHeight;
    if (m_lines <= 0)
        m_lines = 1;
    m_columns = (area.width() - m_leftMargin * 2 - scrollBarWidth) / m_fontWidth;
    if (m_columns <= 0)
        m_columns = 1;

    m_image.resize(m_lines * m_columns);
    m_image.fill(DTerminalChar());
    m_lineFlags.resize(m_lines);
    m_lineFlags.fill(LineDefault);
    m_imageRecreated = true;  // 镜像重建：下次 updateImage 全量重绘（空行 diff 会漏）

    // 同步滚动条范围
    const int totalLines = m_screen->lineCount() + m_screen->historyLineCount();
    const int scrollMax = qMax(0, totalLines - m_lines);
    m_scrollBar->setMaximum(scrollMax);
    m_scrollBar->setPageStep(m_lines);
    m_scrollBar->setValue(qBound(0, m_currentLine, scrollMax));

    updateImage();
}

void DTerminalView::refillImage(int mergedStart, int mergedEnd)
{
    // 两步式：以屏幕列数步幅取图（getImage），
    // 再逐行拷进视图步幅镜像。直接用屏幕步幅写视图
    // 镜像会在 resize 过渡期（view/screen 列数短暂不同步）产生对角线
    // 错位与陈旧行；本方法同时补齐视图超出屏幕的区域（fillUnusedArea 语义）
    const int mergedLines = mergedEnd - mergedStart + 1;
    const int screenColumns = m_screen->columnCount();

    m_windowBuffer.resize(mergedLines * screenColumns);
    m_screen->getImage(m_windowBuffer.data(), m_windowBuffer.size(), mergedStart, mergedEnd);

    const int copyColumns = qMin(m_columns, screenColumns);
    for (int line = 0; line < mergedLines; ++line) {
        const DTerminalChar *source = m_windowBuffer.constData() + line * screenColumns;
        DTerminalChar *target = m_image.data() + line * m_columns;
        for (int column = 0; column < copyColumns; ++column)
            target[column] = source[column];
        // 视图比屏幕宽（过渡期）：行尾补默认字符
        for (int column = copyColumns; column < m_columns; ++column)
            target[column] = DTerminalChar();
    }

    // 窗口越过屏幕末尾的行补默认字符
    for (int line = mergedLines; line < m_lines; ++line) {
        DTerminalChar *target = m_image.data() + line * m_columns;
        for (int column = 0; column < m_columns; ++column)
            target[column] = DTerminalChar();
    }
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

    refillImage(mergedStart, mergedEnd);
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

    // 全量重绘。区域级 diff 重绘在窗口 map/resize/expose 生命周期下
    // 会留下永不重绘的陈旧区域
    // （diff 判"未变化"→ 背部存储丢失的像素无人补画）
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

void DTerminalView::syncOutputTracking()
{
    if (m_screen == nullptr || !m_trackOutput || !m_isAllowScroll)
        return;

    m_currentLine = m_screen->historyLineCount();  // 跟随输出（未节流）
}

void DTerminalView::notifyOutputChanged()
{
    if (m_screen == nullptr)
        return;

    if (m_trackOutput && m_isAllowScroll) {
        const int historyLines = m_screen->historyLineCount();
        m_currentLine = historyLines;  // 跟随输出
    }

    m_screen->resetScrolledLines();

    const int totalLines = m_screen->lineCount() + m_screen->historyLineCount();
    const int scrollMax = qMax(0, totalLines - m_lines);
    m_scrollBar->setMaximum(scrollMax);
    m_scrollBar->setValue(qBound(0, m_currentLine, scrollMax));

    // 首次超屏显示滚动条并保持常显（反复显隐会改变列数引发重排）
    if (m_scrollbarLocation == NoScrollBar && m_lines > 1 && totalLines > m_lines)
        setScrollBarPosition(ScrollBarRight);

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

    // 滚动位置决定是否跟随输出（滚到最底即恢复跟随）
    m_trackOutput = (m_currentLine == scrollMax);

    updateImage();
    processFilters();  // 滚动改变窗口内容，热点需重扫
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

    // 内容区：内容矩形内再扣除边距
    const QRect area = contentsRect();
    m_contentRect = QRect(m_leftMargin + m_leftBaseMargin,
                          m_topMargin + m_topBaseMargin,
                          qMax(0, area.width() - 2 * (m_leftMargin + m_leftBaseMargin)),
                          qMax(0, area.height() - 2 * (m_topMargin + m_topBaseMargin)));

    if (m_centerContents)
        m_contentRect.moveCenter(area.center());

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
    processFilters();  // 列数/行数变化后热点需重扫
    update();

    if (m_resizeTimer->isActive())
        m_resizeTimer->stop();
    m_resizeTimer->start();
    m_userIsResizing = false;
}

QSize DTerminalView::sizeHint() const
{
    // 边距 + 滚动条条带 + 整格内容
    const int scrollBarWidth = m_scrollBar->isHidden()
            ? 0 : m_scrollBar->style()->styleHint(QStyle::SH_ScrollBar_Transient, nullptr, m_scrollBar)
              ? 0 : m_scrollBar->sizeHint().width();
    return QSize(2 * (m_leftMargin + m_leftBaseMargin) + scrollBarWidth + m_columns * m_fontWidth,
                 2 * (m_topMargin + m_topBaseMargin) + m_lines * m_fontHeight);
}

void DTerminalView::setSize(int columns, int lines)
{
    if (m_screen == nullptr)
        return;

    // 尺寸未变直接返回，打断视图-会话尺寸联动环路
    if (m_lines == lines && m_columns == columns)
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
    // 字号限制
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
    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing, s_antialiasText);

    // 调色板槽位：[0]=前景、[1]=背景
    const QColor background = m_colorsInverted
            ? m_palette[0].color : m_palette[1].color;
    const QColor foreground = m_colorsInverted
            ? m_palette[1].color : m_palette[0].color;

    // 只重绘脏区（其余区域由像素滚动/上次绘制保留）
    const QRegion dirty = event->region() & contentsRect();

    // 背景（含透明度混合）：painter 被裁剪到脏区，画整屏即可
    if (m_backgroundImage.isNull()) {
        drawBackground(painter, contentsRect(), background, true);
    } else {
        painter.drawPixmap(rect(), m_backgroundImage);
        QColor blend = background;
        blend.setAlphaF(background.alphaF() * m_opacity);
        painter.fillRect(contentsRect(), blend);
    }

    painter.setBackground(background);
    painter.setPen(foreground);

    // 以整行粒度绘制脏区内容（避免部分单元格重绘残影）
    if (m_columns > 0 && m_lines > 0) {
        for (const QRect &r : dirty) {
            const int startLine = qMax(0, (r.top() - m_topMargin - m_topBaseMargin) / m_fontHeight);
            const int endLine = qMin(m_lines - 1, (r.bottom() - m_topMargin - m_topBaseMargin) / m_fontHeight);
            if (startLine <= endLine)
                // QRect 为 (x, y, w, h) 语义：宽高须按行列数换算，
                // 误用 endLine 作高会漏画最后一行（底行内容永不绘制）
                drawContent(painter, QRect(0, startLine, m_columns, endLine - startLine + 1));
        }
    }

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
        fillColor.setAlphaF(color.alphaF() * m_opacity);
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

        const quint8 lineProperty = m_lineFlags.value(y, quint8(LineDefault));

        // 双宽/双高行的文本缩放（DW 横向 2 倍、
        // DH 上半行纵向 2 倍；下半行无标志、不缩放）
        QTransform textScale;
        if (lineProperty & LineDoubleWidth)
            textScale.scale(2, 1);
        if (lineProperty & LineDoubleHeightTop)
            textScale.scale(1, 2);
        painter.setWorldTransform(textScale, true);

        // GB18030 模式下按字形实测宽度排字，逐 run 累加偏移
        int xOffset = 0;

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
            } else if (cell.codePoint != 0 && cell.codePoint != ' ') {
                buffer[position++] = cell.codePoint;
            }

            // 按相同样式聚合连续片段（BiDi 开启时按书写方向脚本分块）
            const DTerminalChar &styleRef = cell;
            if (cell.codePoint == ' ' && !(lineProperty & LineDoubleWidth)) {
                // 空格段：整段并入单个 run，只画背景不产生文本
                // （也避免超宽字形后的空格造成裁剪）
                while (x + length <= rect.right() && x + length < m_columns) {
                    const DTerminalChar &next = m_image[y * m_columns + x + length];
                    if (next.codePoint != ' ' || next.style != styleRef.style
                            || next.fg != styleRef.fg || next.bg != styleRef.bg)
                        break;
                    length++;
                }
            } else {
                QChar::Script lastScript = position > 0
                        ? QChar::script(charBuffer[0]) : QChar::Script_Unknown;
                while (x + length <= rect.right() && x + length < m_columns) {
                    const DTerminalChar &next = m_image[y * m_columns + x + length];
                    if (next.style != styleRef.style || next.fg != styleRef.fg || next.bg != styleRef.bg)
                        break;
                    if (next.codePoint == 0)
                        break;
                    if (next.style & StyleExtended)
                        break;
                    if (m_bidiEnabled) {
                        const QChar::Script script = QChar::script(next.codePoint);
                        if (!isSameScript(script, lastScript))
                            break;
                        lastScript = script;
                    }
                    buffer[position++] = next.codePoint;
                    length++;
                }
            }

            // 宽字符延续格并入本片段（缩放与裁剪都需要整格宽度）
            if (x + length <= rect.right() && x + length < m_columns
                    && m_image[y * m_columns + x + length].codePoint == 0)
                length++;

            const QString text = QString::fromUcs4(charBuffer.constData(), position);
            QRect textArea;
            if (DCharWidth::gb18030Mode()) {
                // 按字形实测宽度排字，后续片段起点由 xOffset 累加决定
                const QFontMetrics fm(painter.font());
                const int textWidth = fm.size(Qt::TextSingleLine | Qt::TextExpandTabs, text).width();
                textArea = QRect(m_leftMargin + m_leftBaseMargin + xOffset,
                                 m_topMargin + m_topBaseMargin + y * m_fontHeight,
                                 textWidth, m_fontHeight);
                xOffset += textWidth;
            } else {
                textArea = QRect(m_leftMargin + m_leftBaseMargin + x * m_fontWidth,
                                 m_topMargin + m_topBaseMargin + y * m_fontHeight,
                                 m_fontWidth * length, m_fontHeight);
            }

            // 把未变换坐标映射回变换后空间，保证缩放行从正确位置起画
            textArea.moveTopLeft(textScale.inverted().map(textArea.topLeft()));

            drawTextFragment(painter, textArea, text, styleRef);

            x += length - 1;
        }

        // 恢复单宽单高
        painter.setWorldTransform(QTransform(textScale.inverted()), true);

        // 双高上半行由两行组成，跳过下半行
        if ((lineProperty & LineDoubleHeightTop) && y < m_lines - 1)
            y++;
    }
}

void DTerminalView::drawTextFragment(QPainter &painter, const QRect &rect,
                                     const QString &text, const DTerminalChar &style)
{
    painter.save();

    const QColor backgroundColor = resolveColor(style.bg);

    // 与显示背景不同才画底
    if (backgroundColor != painter.background().color())
        drawBackground(painter, rect, backgroundColor, false);

    bool invertCharacterColor = false;
    if (!m_hideCursor && (style.style & StyleCursor)) {
        const QColor cursorForeground = resolveColor(style.fg);
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
    const bool useOverline = (style.style & StyleOverline) || font().overline();

    QFont paintFont = painter.font();
    if (paintFont.bold() != useBold
            || paintFont.underline() != useUnderline
            || paintFont.italic() != useItalic
            || paintFont.strikeOut() != useStrikeOut
            || paintFont.overline() != useOverline) {
        paintFont.setBold(useBold);
        paintFont.setUnderline(useUnderline);
        paintFont.setItalic(useItalic);
        paintFont.setStrikeOut(useStrikeOut);
        paintFont.setOverline(useOverline);
        painter.setFont(paintFont);
    }

    // 前景色（System/Index256/Default/Rgb 四色空间统一查表）
    QColor color = resolveColor(style.fg);

    if (invertCharacterColor) {
        color = painter.background().color();
    }
    painter.setPen(color);

    // 线绘字符自绘
    if (m_drawLineChars && isLineCharString(text)) {
        drawLineCharString(painter, rect.x(), rect.y(), text, style);
    } else if (m_bidiEnabled) {
        painter.drawText(rect, Qt::AlignBottom | Qt::AlignLeft, text);
    } else {
        // 终端无 RTL 布局用例：默认强制 LTR，
        // 但仍允许 RTL 字符按自身方向成形
        painter.drawText(rect, Qt::AlignBottom | Qt::AlignLeft, QChar(0x202D) + text);
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

// 盒线字符段方向位
constexpr uint kLineSegLeft = 1;
constexpr uint kLineSegRight = 2;
constexpr uint kLineSegTop = 4;
constexpr uint kLineSegBottom = 8;
constexpr uint kLineSegAll = kLineSegLeft | kLineSegRight | kLineSegTop | kLineSegBottom;

// 盒线字符过程化绘制：以码点解析上/下/左/右线段，
// 支持单线/粗线/双线/虚线变体；未覆盖码点回退字体渲染。
void DTerminalView::drawLineChar(QPainter &painter, int x, int y, int w, int h,
                                 uint codePoint, bool bold)
{
    uint segments = 0;
    bool heavy = false;
    bool doubled = false;
    bool dashed = false;

    switch (codePoint) {
    // 单线
    case 0x2500: segments = kLineSegLeft | kLineSegRight; break;            // ─
    case 0x2502: segments = kLineSegTop | kLineSegBottom; break;            // │
    case 0x250C: segments = kLineSegRight | kLineSegBottom; break;          // ┌
    case 0x2510: segments = kLineSegLeft | kLineSegBottom; break;           // ┐
    case 0x2514: segments = kLineSegRight | kLineSegTop; break;             // └
    case 0x2518: segments = kLineSegLeft | kLineSegTop; break;              // ┘
    case 0x251C: segments = kLineSegTop | kLineSegBottom | kLineSegRight; break;  // ├
    case 0x2524: segments = kLineSegTop | kLineSegBottom | kLineSegLeft; break;   // ┤
    case 0x252C: segments = kLineSegLeft | kLineSegRight | kLineSegBottom; break; // ┬
    case 0x2534: segments = kLineSegLeft | kLineSegRight | kLineSegTop; break;    // ┴
    case 0x253C: segments = kLineSegAll; break;                            // ┼
    // 粗线
    case 0x2501: segments = kLineSegLeft | kLineSegRight; heavy = true; break;    // ━
    case 0x2503: segments = kLineSegTop | kLineSegBottom; heavy = true; break;    // ┃
    case 0x250F: segments = kLineSegRight | kLineSegBottom; heavy = true; break;  // ┏
    case 0x2513: segments = kLineSegLeft | kLineSegBottom; heavy = true; break;   // ┓
    case 0x2517: segments = kLineSegRight | kLineSegTop; heavy = true; break;     // ┗
    case 0x251B: segments = kLineSegLeft | kLineSegTop; heavy = true; break;      // ┛
    case 0x2523: segments = kLineSegTop | kLineSegBottom | kLineSegRight; heavy = true; break;  // ┣
    case 0x252B: segments = kLineSegTop | kLineSegBottom | kLineSegLeft; heavy = true; break;   // ┫
    case 0x2533: segments = kLineSegLeft | kLineSegRight | kLineSegBottom; heavy = true; break; // ┳
    case 0x253B: segments = kLineSegLeft | kLineSegRight | kLineSegTop; heavy = true; break;    // ┻
    case 0x254B: segments = kLineSegAll; heavy = true; break;               // ╋
    // 双线
    case 0x2550: segments = kLineSegLeft | kLineSegRight; doubled = true; break;           // ═
    case 0x2551: segments = kLineSegTop | kLineSegBottom; doubled = true; break;           // ║
    case 0x2554: segments = kLineSegRight | kLineSegBottom; doubled = true; break;         // ╔
    case 0x2557: segments = kLineSegLeft | kLineSegBottom; doubled = true; break;          // ╗
    case 0x255A: segments = kLineSegRight | kLineSegTop; doubled = true; break;            // ╚
    case 0x255D: segments = kLineSegLeft | kLineSegTop; doubled = true; break;             // ╝
    case 0x2560: segments = kLineSegTop | kLineSegBottom | kLineSegRight; doubled = true; break;  // ╠
    case 0x2563: segments = kLineSegTop | kLineSegBottom | kLineSegLeft; doubled = true; break;   // ╣
    case 0x2566: segments = kLineSegLeft | kLineSegRight | kLineSegBottom; doubled = true; break; // ╦
    case 0x2569: segments = kLineSegLeft | kLineSegRight | kLineSegTop; doubled = true; break;    // ╩
    case 0x256C: segments = kLineSegAll; doubled = true; break;              // ╬
    // 虚线
    case 0x2504: segments = kLineSegLeft | kLineSegRight; dashed = true; break;  // ┄
    case 0x2506: segments = kLineSegTop | kLineSegBottom; dashed = true; break;  // ┆
    default:
        // 未覆盖的盒线码点回退字体渲染
        painter.drawText(QRect(x, y, w, h), Qt::AlignCenter, QString(QChar(codePoint)));
        return;
    }

    Q_UNUSED(bold)

    const qreal cx = x + w / 2.0;
    const qreal cy = y + h / 2.0;

    QPen pen = painter.pen();
    if (heavy)
        pen.setWidth(3);
    else
        pen.setWidth(1);
    if (dashed)
        pen.setStyle(Qt::DashLine);
    painter.setPen(pen);

    if (doubled) {
        const qreal offset = qMax(1.0, h / 8.0);
        if (segments & (kLineSegLeft | kLineSegRight)) {
            painter.drawLine(QPointF(x, cy - offset), QPointF(x + w, cy - offset));
            painter.drawLine(QPointF(x, cy + offset), QPointF(x + w, cy + offset));
        }
        if (segments & (kLineSegTop | kLineSegBottom)) {
            painter.drawLine(QPointF(cx - offset, y), QPointF(cx - offset, y + h));
            painter.drawLine(QPointF(cx + offset, y), QPointF(cx + offset, y + h));
        }
        return;
    }

    if (segments & (kLineSegLeft | kLineSegRight))
        painter.drawLine(QPointF(x, cy), QPointF(x + w, cy));
    if (segments & (kLineSegTop | kLineSegBottom))
        painter.drawLine(QPointF(cx, y), QPointF(cx, y + h));
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

    switch (m_cursorShape) {
    case DCursorShape::Block:
        if (hasFocus()) {
            painter.fillRect(cursorRect, m_cursorColor.isValid() ? m_cursorColor : foregroundColor);
            if (!m_cursorColor.isValid())
                invertCharacterColor = true;
        } else {
            // 失焦画外框，调整笔宽使外框完全落在格内
            const qreal penWidth = qMax(1, painter.pen().width());
            painter.drawRect(cursorRect.adjusted(penWidth / 2, penWidth / 2,
                                                 -penWidth / 2, -penWidth / 2));
        }
        break;
    case DCursorShape::Underline:
        // 不区分焦点：整数像素 1px 高矩形，避免端点抗锯齿残留
        {
            const QRect intRect = cursorRect.toAlignedRect();
            const QColor lineColor = m_cursorColor.isValid() ? m_cursorColor : foregroundColor;
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing, false);
            painter.setPen(Qt::NoPen);
            painter.fillRect(QRect(intRect.left(), intRect.bottom(), intRect.width(), 1), lineColor);
            painter.restore();
        }
        break;
    case DCursorShape::IBeam:
        // 不区分焦点：整数像素 1px 宽竖线
        {
            const QRect intRect = cursorRect.toAlignedRect();
            const QColor lineColor = m_cursorColor.isValid() ? m_cursorColor : foregroundColor;
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing, false);
            painter.setPen(Qt::NoPen);
            painter.fillRect(QRect(intRect.left(), intRect.top(), 1, intRect.height()), lineColor);
            painter.restore();
        }
        break;
    case DCursorShape::BoldUnderline:
        {
            const QRect intRect = cursorRect.toAlignedRect();
            const int thickness = qMin(4, intRect.height());
            const int topY = intRect.bottom() - (thickness - 1);
            if (hasFocus()) {
                painter.fillRect(QRect(intRect.left(), topY, intRect.width(), thickness),
                                 m_cursorColor.isValid() ? m_cursorColor : foregroundColor);
            } else {
                painter.fillRect(QRect(intRect.left(), topY, intRect.width(), thickness),
                                 painter.pen().color());
            }
        }
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
    // 按实际字符宽度布局（至少 1 格）
    const int width = qMax(1, DTerminalUtil::displayWidth(m_preedit.preeditString));
    return QRect(m_leftMargin + m_leftBaseMargin + cursor.x() * m_fontWidth,
                 m_topMargin + m_topBaseMargin + cursor.y() * m_fontHeight,
                 m_fontWidth * width, m_fontHeight);
}

void DTerminalView::updateCursor()
{
    if (m_screen == nullptr)
        return;

    const QPoint cursor = cursorPosition();
    if (cursor.y() < 0 || cursor.y() >= m_lines)
        return;
    update(imageToWidget(QRect(cursor, QSize(1, 1))));
}

QColor DTerminalView::resolveColor(const DTerminalCellColor &color) const
{
    switch (color.space) {
    case ColorSpaceSystem:
        return m_palette[2 + color.primary + (color.secondary ? 10 : 0)].color;
    case ColorSpaceIndex256:
        return mapIndex256(color.primary, m_palette);
    case ColorSpaceDefault:
        // 粗体默认色走亮色变体
        return m_palette[(color.primary == kDefaultForeIndex ? 0 : 1)
                         + (color.secondary ? 10 : 0)].color;
    case ColorSpaceRgb:
    default:
        return QColor(color.primary, color.secondary, color.tertiary);
    }
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

    // 画笔取鼠标所在字符的前景色，下划线颜色与文字一致
    int cursorLine = 0, cursorColumn = 0;
    getCharacterPosition(mapFromGlobal(QCursor::pos()), cursorLine, cursorColumn);
    if (cursorLine >= 0 && cursorLine < m_lines && cursorColumn >= 0 && cursorColumn < m_columns)
        painter.setPen(QPen(resolveColor(m_image[cursorLine * m_columns + cursorColumn].fg)));

    // 只给悬停中的链接画实线下划线（检查区四周
    // 收 1px，防止指针停在链接边界上时仍显示下划线）
    bool hoveringLink = false;
    const QList<DTextFilter::Spot *> spots = m_scanner->spots();
    for (DTextFilter::Spot *spot : spots) {
        if (spot->kind() != DTextFilter::Spot::KindLink)
            continue;

        if (!spotRegion(spot, true).contains(mapFromGlobal(QCursor::pos())))
            continue;

        // 逐行画下划线：行尾空白裁剪、右缘 -1 不压线邻格
        for (int line = spot->startLine(); line <= spot->endLine(); ++line) {
            int startColumn = 0;
            int endColumn = m_columns - 1;
            while (QChar(m_image[line * m_columns + endColumn].codePoint).isSpace() && endColumn > 0)
                --endColumn;
            ++endColumn;  // 行尾第一个空白字符（半开右缘）

            if (line == spot->startLine())
                startColumn = spot->startColumn();
            if (line == spot->endLine())
                endColumn = spot->endColumn();

            QRect r;
            r.setCoords(startColumn * m_fontWidth + 1 + m_leftMargin + m_leftBaseMargin,
                        line * m_fontHeight + 1 + m_topMargin + m_topBaseMargin,
                        endColumn * m_fontWidth - 1 + m_leftMargin + m_leftBaseMargin,
                        (line + 1) * m_fontHeight - 1 + m_topMargin + m_topBaseMargin);

            // 基线：字形坐落的不可见线；下划线画在其下方 underlinePos 处
            const QFontMetrics metrics(font());
            const int baseline = r.bottom() - metrics.descent();
            const int underlinePos = baseline + metrics.underlinePos();
            painter.drawLine(r.left(), underlinePos, r.right(), underlinePos);
            hoveringLink = true;
        }
    }

    // 悬停链接且按住 Ctrl 才切换小手，否则 IBeam
    if (hoveringLink && (QApplication::queryKeyboardModifiers() & Qt::ControlModifier)) {
        if (cursor().shape() != Qt::PointingHandCursor)
            setCursor(Qt::PointingHandCursor);
    } else if (cursor().shape() != Qt::IBeamCursor) {
        setCursor(Qt::IBeamCursor);
    }
}

QRegion DTerminalView::spotRegion(DTextFilter::Spot *spot, bool inset) const
{
    // 热点单元格区域（列按半开区间）
    QRegion region;
    QRect r;
    if (spot->startLine() == spot->endLine()) {
        r.setCoords(spot->startColumn() * m_fontWidth + m_leftMargin + m_leftBaseMargin,
                    spot->startLine() * m_fontHeight + m_topMargin + m_topBaseMargin,
                    spot->endColumn() * m_fontWidth + m_leftMargin + m_leftBaseMargin,
                    (spot->endLine() + 1) * m_fontHeight - 1 + m_topMargin + m_topBaseMargin);
        region |= r;
    } else {
        r.setCoords(spot->startColumn() * m_fontWidth + m_leftMargin + m_leftBaseMargin,
                    spot->startLine() * m_fontHeight + m_topMargin + m_topBaseMargin,
                    m_columns * m_fontWidth - 1 + m_leftMargin + m_leftBaseMargin,
                    (spot->startLine() + 1) * m_fontHeight - 1 + m_topMargin + m_topBaseMargin);
        region |= r;
        for (int line = spot->startLine() + 1; line < spot->endLine(); ++line) {
            r.setCoords(m_leftMargin + m_leftBaseMargin,
                        line * m_fontHeight + m_topMargin + m_topBaseMargin,
                        m_columns * m_fontWidth - 1 + m_leftMargin + m_leftBaseMargin,
                        (line + 1) * m_fontHeight - 1 + m_topMargin + m_topBaseMargin);
            region |= r;
        }
        r.setCoords(m_leftMargin + m_leftBaseMargin,
                    spot->endLine() * m_fontHeight + m_topMargin + m_topBaseMargin,
                    spot->endColumn() * m_fontWidth + m_leftMargin + m_leftBaseMargin,
                    (spot->endLine() + 1) * m_fontHeight - 1 + m_topMargin + m_topBaseMargin);
        region |= r;
    }

    if (inset) {
        QRegion shrunk;
        for (const QRect &rect : region)
            shrunk |= rect.adjusted(1, 1, -1, -1);
        return shrunk;
    }
    return region;
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

    refillImage(mergedStart, mergedEnd);
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
    // 粘贴取消当前选区；选区色在 getImage 时
    // 套用，清除后必须重取图，否则旧高亮残留
    if (m_screen != nullptr) {
        m_screen->clearSelection();
        updateImage();
    }
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
    // 粘贴取消当前选区；选区色在 getImage 时
    // 套用，清除后必须重取图，否则旧高亮残留
    if (m_screen != nullptr) {
        m_screen->clearSelection();
        updateImage();
    }
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

    // 选区拖出窗口上下边界时自动滚动视图（按格计算）
    if (pos.y() > m_lines - 1)
        m_scrollBar->setValue(m_scrollBar->value() + pos.y() - m_lines + 1);
    else if (pos.y() < 0)
        m_scrollBar->setValue(m_scrollBar->value() + pos.y() - 1);

    // 钳制到可见区
    const QPoint here(qBound(0, pos.x(), m_columns - 1), qBound(0, pos.y(), m_lines - 1));

    QPoint start = m_selectionInitial;
    QPoint end = here;
    const QPoint anchor = m_selectionInitial;

    if (m_wordSelectionMode) {
        // 词选：双向找词边界，可跨软换行行
        const bool leftNotRight = (here.y() < anchor.y())
                || (here.y() == anchor.y() && here.x() < anchor.x());
        QPoint left = leftNotRight ? here : anchor;
        QPoint right = leftNotRight ? anchor : here;

        // 向左扩展
        int i = left.y() * m_columns + left.x();
        const QChar selClass = classifyChar(QChar(m_image[i].codePoint));
        while ((left.x() > 0
                || (left.y() > 0 && (m_lineFlags.value(left.y() - 1, quint8(LineDefault)) & LineWrapped)))
               && classifyChar(QChar(m_image[i - 1].codePoint)) == selClass) {
            i--;
            if (left.x() > 0)
                left.rx()--;
            else {
                left.rx() = m_columns - 1;
                left.ry()--;
            }
        }

        // 向右扩展
        i = right.y() * m_columns + right.x();
        while ((right.x() < m_columns - 1
                || (right.y() < m_lines - 1 && (m_lineFlags.value(right.y(), quint8(LineDefault)) & LineWrapped)))
               && classifyChar(QChar(m_image[i + 1].codePoint)) == selClass) {
            i++;
            if (right.x() < m_columns - 1)
                right.rx()++;
            else {
                right.rx() = 0;
                right.ry()++;
            }
        }

        start = left;
        end = right;
    } else if (m_lineSelectionMode) {
        // 行选：沿软换行链扩展整行
        const bool aboveNotBelow = (here.y() < anchor.y());
        QPoint above = aboveNotBelow ? here : anchor;
        QPoint below = aboveNotBelow ? anchor : here;

        while (above.y() > 0 && (m_lineFlags.value(above.y() - 1, quint8(LineDefault)) & LineWrapped))
            above.ry()--;
        while (below.y() < m_lines - 1 && (m_lineFlags.value(below.y(), quint8(LineDefault)) & LineWrapped))
            below.ry()++;

        above.setX(0);
        below.setX(m_columns - 1);

        start = above;
        end = below;
    }

    m_selectionInitial = start;
    m_selectionCurrent = end;
    m_selectionState = 2;

    // 屏幕坐标（窗口）→ 全局坐标（含历史）
    const int windowTopLine = m_currentLine;
    m_screen->setSelectionStart(start.x(), start.y() + windowTopLine,
                                m_columnSelectionMode);
    m_screen->setSelectionEnd(end.x(), end.y() + windowTopLine);

    // 选区状态跟踪
    int startLine = 0, startColumn = 0, endLine = 0, endColumn = 0;
    m_screen->getSelectionStart(startColumn, startLine);
    m_screen->getSelectionEnd(endColumn, endLine);
    m_selStartLine = startLine;
    m_selStartColumn = startColumn;
    m_selEndLine = endLine;
    m_selEndColumn = endColumn;
    m_selBegin = true;

    selectionChanged();
    // 选区色在 getImage 时套用，必须重取图并按 diff 重绘（只 update 会画旧镜像）
    updateImage();
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

        // Ctrl+点击定位光标
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
                // 程序接管鼠标：报告事件（行号换算滚动偏移）
                emit mouseEvent(0, charColumn + 1,
                                charLine + 1 + m_scrollBar->value() - m_scrollBar->maximum(), 0);
            } else {
                m_screen->clearSelection();
                // 选区色在 getImage 时套用：清除后必须重取图，否则旧高亮残留
                updateImage();
                const QPoint pos(charColumn, charLine);
                m_selectionInitial = pos;
                m_selectionCurrent = pos;
                initSelectionState();
            }

            // 点击链接即激活热点（打开与否由上层 onUrlActivated
            // 的 Ctrl 门槛决定）；三击全选不激活
            if (!m_possibleTripleClick && m_scanner != nullptr) {
                DTextFilter::Spot *spot = m_scanner->spotAt(charLine, charColumn);
                if (spot != nullptr && spot->kind() == DTextFilter::Spot::KindLink)
                    spot->activate(QLatin1String("click-action"));
            }
        }
    } else if (event->button() == Qt::MiddleButton) {
        if (m_usesMouse) {
            int charLine = 0, charColumn = 0;
            getCharacterPosition(event->pos(), charLine, charColumn);
            emit mouseEvent(1, charColumn + 1,
                            charLine + 1 + m_scrollBar->value() - m_scrollBar->maximum(), 0);
        } else {
            pasteSelection();
        }
    } else if (event->button() == Qt::RightButton) {
        if (m_usesMouse && !(event->modifiers() & Qt::ShiftModifier)) {
            int charLine = 0, charColumn = 0;
            getCharacterPosition(event->pos(), charLine, charColumn);
            emit mouseEvent(2, charColumn + 1,
                            charLine + 1 + m_scrollBar->value() - m_scrollBar->maximum(), 0);
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

    // 链接热点悬停：跟踪悬停区并触发重绘（下划线与指针在 paint 时按全局
    // 鼠标位置现算；mouseMove 只负责重绘触发）
    if (m_scanner != nullptr) {
        DTextFilter::Spot *spot = m_scanner->spotAt(charLine, charColumn);
        const QRegion previousHotspotArea = m_mouseOverSpotArea;
        m_mouseOverSpotArea = QRegion();
        if (spot != nullptr && spot->kind() == DTextFilter::Spot::KindLink)
            m_mouseOverSpotArea = spotRegion(spot, false);
        update(m_mouseOverSpotArea | previousHotspotArea);
    }

    // 程序接管鼠标时的移动上报
    if (m_usesMouse && !(event->buttons() & Qt::LeftButton)) {
        emit mouseEvent(3, charColumn + 1,
                        charLine + 1 + m_scrollBar->value() - m_scrollBar->maximum(), 1);
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
            emit mouseEvent(0, charColumn + 1,
                            charLine + 1 + m_scrollBar->value() - m_scrollBar->maximum(), 2);
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

    // 程序接管鼠标：把双击转成第二次单击上报（首次单击已由 press 上报）
    if (m_usesMouse && !(event->modifiers() & Qt::ShiftModifier)) {
        emit mouseEvent(0, charColumn + 1,
                        charLine + 1 + m_scrollBar->value() - m_scrollBar->maximum(), 0);
        return;
    }

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

    // 窗口激活且有焦点时，Ctrl+滚轮交给父级处理（缩放等）
    if (isActiveWindow() && hasFocus() && event->modifiers() == Qt::ControlModifier) {
        QWidget::wheelEvent(event);
        return;
    }

    // 滚轮滚动时初始化键盘选择状态
    initKeyboardSelection();

    if (event->angleDelta().x() != 0)
        return;

    const int delta = event->angleDelta().y();
    const QPoint pos = event->position().toPoint();

    if (!m_usesMouse && m_scrollBar->maximum() > 0) {
        // 程序未接管鼠标：滚轮滚动视图（终端自处理鼠标，
        // 与 m_usesMouse 相反）
        const int steps = delta / 120;
        scrollBy(ScrollLines, -steps);
    } else if (!m_usesMouse && m_alternateScreen && m_alternateScrolling) {
        // 无历史可滚 + 备用屏：滚轮转为上下键（每 5 度一行）
        const int key = delta > 0 ? Qt::Key_Up : Qt::Key_Down;
        const int wheelDegrees = delta / 8;
        const int linesToScroll = qAbs(wheelDegrees) / 5;
        QKeyEvent keyScrollEvent(QEvent::KeyPress, key, Qt::NoModifier);
        for (int i = 0; i < linesToScroll; ++i)
            emit keyPressed(&keyScrollEvent);
    } else if (m_usesMouse) {
        // 程序接管鼠标：上报按钮 4/5（行号换算滚动偏移）
        int charLine = 0, charColumn = 0;
        getCharacterPosition(pos, charLine, charColumn);
        emit mouseEvent(delta > 0 ? 4 : 5, charColumn + 1,
                        charLine + 1 + m_scrollBar->value() - m_scrollBar->maximum(), 0);
    }
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
            // Shift+上下键：滚动历史并重新初始化选择锚点
            initKeyboardSelection();
            scrollBy(ScrollLines, -1);
            emitKeyPressSignal = false;
        } else if (event->key() == Qt::Key_Down) {
            initKeyboardSelection();
            scrollBy(ScrollLines, 1);
            emitKeyPressSignal = false;
        } else if (event->key() == Qt::Key_End) {
            scrollToEnd();
            emitKeyPressSignal = false;
        } else if (event->key() == Qt::Key_Home) {
            initKeyboardSelection();
            scrollTo(0);
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

    // 按键重启光标闪烁并复位相位
    if (m_hasBlinkingCursor) {
        m_blinkCursorTimer->start(QApplication::cursorFlashTime() / 2);
        if (m_cursorBlinking)
            blinkCursorEvent();
        else
            m_cursorBlinking = false;
    }

    if (emitKeyPressSignal)
        emit keyPressed(event);

    // 按键滚动：粘贴滚动模式为
    // "移至末尾"时，普通按键后滚到底部——输入行滚出可见区后回车即回到底
    switch (m_motionAfterPasting) {
    case MoveStartScreenWindow:
        scrollTo(0);
        break;
    case MoveEndScreenWindow:
        if (!(event->key() == Qt::Key_Control
                || event->key() == Qt::Key_Shift
                || event->key() == Qt::Key_Alt)) {
            scrollToEnd();
        }
        break;
    case NoMoveScreenWindow:
        break;
    }

    event->accept();
}

bool DTerminalView::handleShortcutOverride(QKeyEvent *event)
{
    const int modifiers = event->modifiers();

    // 单修饰键组合交由宿主裁决是否覆盖
    if (modifiers != Qt::NoModifier) {
        int modifierCount = 0;
        unsigned int currentModifier = Qt::ShiftModifier;
        while (currentModifier <= Qt::KeypadModifier) {
            if (modifiers & currentModifier)
                modifierCount++;
            currentModifier <<= 1;
        }
        if (modifierCount < 2) {
            bool override = false;
            emit shortcutOverrideCheck(event, override);
            if (override) {
                event->accept();
                return true;
            }
        }
    }

    // 终端必需的按键覆盖应用快捷键（清单取自 QLineEdit::event）
    const int keyCode = event->key() | modifiers;
    switch (keyCode) {
    case Qt::Key_Tab:
    case Qt::Key_Backtab:  // Shift+Tab，vim 补全等需要
    case Qt::Key_Tab | Qt::ShiftModifier:
    case Qt::Key_Delete:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_Backspace:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Escape:
        event->accept();
        return true;
    default:
        return false;
    }
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

    // 只重绘含闪烁字符的行
    QRegion blinkRegion;
    for (int row = 0; row < m_lines; ++row) {
        bool hasBlink = false;
        for (int col = 0; col < m_columns; ++col) {
            if (m_image[row * m_columns + col].style & StyleBlink) {
                hasBlink = true;
                break;
            }
        }
        if (hasBlink)
            blinkRegion |= imageToWidget(QRect(0, row, m_columns - 1, row));
    }
    if (!blinkRegion.isEmpty())
        update(blinkRegion);
}

void DTerminalView::blinkCursorEvent()
{
    if (!m_hasBlinkingCursor)
        return;

    m_cursorBlinking = !m_cursorBlinking;
    updateCursor();
}

void DTerminalView::setBlinkingCursor(bool blink)
{
    m_hasBlinkingCursor = blink;

    if (blink && !m_blinkCursorTimer->isActive())
        m_blinkCursorTimer->start(QApplication::cursorFlashTime() / 2);

    if (!blink && m_blinkCursorTimer->isActive()) {
        m_blinkCursorTimer->stop();
        // 处于隐藏相位时翻回，保证关闭闪烁后光标立即可见
        if (m_cursorBlinking)
            blinkCursorEvent();
        else
            m_cursorBlinking = false;
    }

    // 按当前焦点同步定时器，
    // 避免"设置光标闪烁后非焦点光标也在闪烁"
    if (hasFocus())
        focusInEvent(nullptr);
    else
        focusOutEvent(nullptr);
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

    switch (position) {
    case NoScrollBar:
        m_scrollBar->hide();
        break;
    case ScrollBarLeft:
        m_scrollBar->show();
        break;
    case ScrollBarRight:
    default:
        m_scrollBar->show();
        break;
    }

    // 重新计算几何（含滚动条手动定位与列数让位）并把新尺寸同步给屏幕模型
    updateImageSize();
    propagateSize();
    processFilters();  // 列数/行数变化后热点需重扫
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
    m_opacity = qBound<qreal>(0.0, opacity, 1.0);
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
    // 背景自填充的擦除色随主题同步（换配色即时生效）
    QPalette selfPal = QWidget::palette();
    selfPal.setBrush(QPalette::Window, m_palette[1].color);
    QWidget::setPalette(selfPal);
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

    // 主题切换时同步滚动条调色板（视图自身调色板被配色
    // 冻结，滚动条跟随应用调色板才随浅/深主题重绘，否则右侧留旧色）
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::ApplicationPaletteChange)
        m_scrollBar->setPalette(QApplication::palette());

    return QWidget::event(event);
}

void DTerminalView::focusInEvent(QFocusEvent *event)
{
    emit focusInTerminal();
    QWidget::focusInEvent(event);

    // 失焦时停掉的闪烁定时器按需恢复
    if (m_hasBlinkingCursor)
        m_blinkCursorTimer->start();
    updateCursor();

    if (m_hasBlinker)
        m_blinkTimer->start();
}

void DTerminalView::focusOutEvent(QFocusEvent *event)
{
    emit focusLeftTerminal();
    QWidget::focusOutEvent(event);

    // 失焦即停止闪烁并复位隐藏相位（光标保持可见的
    // 失焦样式，不再闪烁）
    m_cursorBlinking = false;
    updateCursor();

    m_blinkCursorTimer->stop();
    if (m_blinking)
        blinkEvent();
    m_blinkTimer->stop();
}

bool DTerminalView::focusNextPrevChild(bool next)
{
    Q_UNUSED(next)
    // 禁用 Tab/Shift+Tab 焦点切换：QWidget::event 会先于
    // keyPressEvent 拦截 Tab 走焦点链，返回 false 让按键落到终端
    return false;
}

void DTerminalView::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

void DTerminalView::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

// ---- 手势 ----

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
    switch (gesture->state()) {
    case Qt::GestureStarted:
        m_gestureAction = GesturePan;
        break;
    case Qt::GestureUpdated:
        // 内容随手指移动（纵轴）
        slideGesture(gesture->delta().y());
        break;
    case Qt::GestureFinished:
    case Qt::GestureCanceled:
        m_gestureAction = GestureNone;
        break;
    default:
        break;
    }
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
    if (gesture->state() != Qt::GestureFinished)
        return;

    const QSwipeGesture::SwipeDirection direction = gesture->verticalDirection();
    if (direction == QSwipeGesture::NoDirection)
        return;

    // 惯性滑动：手指下扫内容上移（速度经缓动衰减；Qt 无手势速度接口，取固定初速）
    m_inertiaVelocity = (direction == QSwipeGesture::Down ? 1 : -1) * m_fontHeight * 4;
    if (!m_inertiaTimer->isActive())
        m_inertiaTimer->start();
    m_gestureAction = GestureNone;
}

void DTerminalView::slideGesture(qreal diff)
{
    if (qAbs(diff) < 1.0)
        return;

    // 像素差 → 行数；diff 为正（手指下移）时向上滚动显示历史
    const int lines = qMax(1, static_cast<int>(qAbs(diff) / m_fontHeight));
    scrollBy(ScrollLines, diff > 0 ? -lines : lines);
}
