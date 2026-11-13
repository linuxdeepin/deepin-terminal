// Copyright (C) 2026 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DTERMINALVIEW_H
#define DTERMINALVIEW_H

#include "dterminalutil.h"
#include "dtextfilter.h"

#include <QGesture>
#include <QGestureEvent>
#include <QPanGesture>
#include <QPinchGesture>
#include <QSwipeGesture>
#include <QTapAndHoldGesture>
#include <QTapGesture>
#include <QWidget>

class QScrollBar;
class QTimer;
class QLabel;
class QDragEnterEvent;
class QDropEvent;

/*******************************************************************************
 1. @类名:    DTerminalView
 2. @作者:    ut001213 liuzhangjian
 3. @日期:    2026-11-13
 4. @说明:    终端显示部件（自研实现）。渲染屏幕图像、处理键盘/
            鼠标/触控输入、选区与剪贴板、滚动条与输出跟踪。
            整合历史实现中"显示部件 + 屏幕窗口"两层：
            滚动窗口模型内置于本类。
            保留 deepin 自有行为：触控手势（捏合/滑动/惯性）、
            Ctrl+点击定位光标、输出滚动许可、选区状态跟踪。
*******************************************************************************/
class DTerminalView : public QWidget
{
    Q_OBJECT
public:
    /** 键盘光标形状（与配置/解析器取值对齐） */
    enum class DCursorShape {
        Block = 0,
        Underline = 1,
        IBeam = 2,
        BoldUnderline = 3
    };

    /** 三击选择模式 */
    enum TripleClickMode {
        SelectWholeLine,
        SelectForwardsFromCursor
    };

    /** 滚动条位置 */
    enum ScrollBarPosition {
        NoScrollBar = 0,
        ScrollBarLeft = 1,
        ScrollBarRight = 2
    };

    /** 粘贴后的滚动动作 */
    enum MotionAfterPasting {
        NoMoveScreenWindow = 0,
        MoveStartScreenWindow = 1,
        MoveEndScreenWindow = 2
    };

    /** 响铃模式 */
    enum BellMode {
        SystemBeepBell = 0,
        NotifyBell = 1,
        VisualBell = 2,
        NoBell = 3
    };

    /** 相对滚动单位 */
    enum RelativeScrollMode {
        ScrollLines,
        ScrollPages
    };

    explicit DTerminalView(QWidget *parent = nullptr);
    ~DTerminalView() override;

    // ---- 外观 ----
    void setPalette(const DTerminalPaletteEntry *table);
    const DTerminalPaletteEntry *palette() const { return m_palette; }
    void setRandomSeed(uint seed) { m_randomSeed = seed; }
    uint randomSeed() const { return m_randomSeed; }
    void setOpacity(qreal opacity);
    void setBackgroundImage(const QString &imagePath);
    void setScrollBarPosition(ScrollBarPosition position);
    void setScroll(int cursor, int lines);
    void scrollToEnd();
    void setBlinkingCursor(bool blink);
    bool blinkingCursor() const { return m_hasBlinkingCursor; }
    void setBlinkingTextEnabled(bool enable) { m_allowBlinkingText = enable; }
    void setTripleClickMode(TripleClickMode mode) { m_tripleClickMode = mode; }
    void setLineSpacing(uint spacing);
    void setMargin(int margin);
    int margin() const { return m_margin; }
    void setCursorShape(DCursorShape shape) { m_cursorShape = shape; }
    DCursorShape cursorShape() const { return m_cursorShape; }
    void setCursorColor(bool useForegroundColor, const QColor &color);
    QColor cursorColor() const { return m_cursorColor; }
    void setWordCharacters(const QString &wordChars) { m_wordCharacters = wordChars; }
    QString wordCharacters() const { return m_wordCharacters; }
    void setBellMode(int mode) { m_bellMode = mode; }
    void setFont(const QFont &font);
    QFont terminalFont() const { return font(); }
    void setVTFont(const QFont &font);
    static void setAntialias(bool antialias) { s_antialiasText = antialias; }
    void setDrawLineChars(bool drawLineChars) { m_drawLineChars = drawLineChars; }
    void setBoldIntense(bool value) { m_boldIntense = value; }
    void setTerminalSizeHint(bool on) { m_terminalSizeHint = on; }
    void setTerminalSizeStartup(bool on) { m_terminalSizeStartup = on; }
    void setBidiEnabled(bool enable) { m_bidiEnabled = enable; }
    bool isBidiEnabled() const { return m_bidiEnabled; }
    void setMotionAfterPasting(int action) { m_motionAfterPasting = action; }
    int motionAfterPasting() const { return m_motionAfterPasting; }
    void setFlowControlWarningEnabled(bool enabled) { m_flowControlWarningEnabled = enabled; }
    void outputSuspended(bool suspended);
    void setUsesMouse(bool uses);
    bool usesMouse() const { return m_usesMouse; }
    void setAlternateScrolling(bool enable) { m_alternateScrolling = enable; }
    void setBracketedPasteMode(bool mode) { m_bracketedPasteMode = mode; }
    bool bracketedPasteMode() const { return m_bracketedPasteMode; }
    void bell(const QString &message);
    void setBackgroundColor(const QColor &color);
    void setForegroundColor(const QColor &color);
    void setHideCursor(bool hide) { m_hideCursor = hide; }
    void setCursorPositionEnable(bool enable) { m_cursorPositionSetEnable = enable; }
    bool isAllowScrollOnOutput() const { return m_isAllowScroll; }
    void setAllowScrollOnOutput(bool allow) { m_isAllowScroll = allow; }

    // ---- 屏幕绑定 ----
    void setScreen(DTerminalScreen *screen);
    DTerminalScreen *screen() const { return m_screen; }

    // ---- 滚动窗口 ----
    int currentLine() const { return m_currentLine; }
    int windowLineCount() const { return m_windowLines; }
    int windowColumnCount() const;
    void setWindowLineCount(int lines);
    void scrollBy(RelativeScrollMode mode, int amount);
    void scrollTo(int line);
    bool atEndOfOutput() const;
    void setTrackOutput(bool track) { m_trackOutput = track; }
    bool trackOutput() const { return m_trackOutput; }
    QPoint cursorPosition() const;

    // ---- 尺寸 ----
    int lineCount() const { return m_lines; }
    int columnCount() const { return m_columns; }
    int fontHeight() const { return m_fontHeight; }
    int fontWidth() const { return m_fontWidth; }
    void setSize(int columns, int lines);
    void setFixedSize(int columns, int lines);
    QSize sizeHint() const override;

    // ---- 过滤 ----
    void setScanner(DTerminalTextScanner *scanner) { m_scanner = scanner; }
    void processFilters();
    QList<QAction *> filterActions(const QPoint &position);

    // ---- 选区与映射 ----
    void setSelection(const QString &text);
    void setSelectionAll();
    /** 粘贴模式文本包裹（\033[200~ / \033[201~） */
    void bracketText(QString &text);
    void getCharacterPosition(const QPoint &widgetPoint, int &line, int &column) const;
    QScrollBar *scrollBar() { return m_scrollBar; }

    void setSessionId(int sessionId) { m_sessionId = sessionId; }

public slots:
    void updateImage();
    void updateLineProperties();
    void copyClipboard();
    void pasteClipboard();
    void pasteSelection();
    void notifyOutputChanged();
    void selectionChanged();
    void selectionCleared();

signals:
    void keyPressed(QKeyEvent *event);
    void mouseEvent(int button, int column, int line, int eventType);
    void fontMetricChanged(int height, int width);
    void contentSizeChanged(int height, int width);
    void cursorPositionChanged(int count);
    void contextMenuRequested(const QPoint &position);
    void shortcutOverrideCheck(QKeyEvent *event, bool &override);
    void busySelecting(bool busy);
    void sendStringToEmulation(const char *text);
    void copyAvailable(bool available);
    void focusInTerminal();
    void focusLeftTerminal();
    void leftMouseClicked();
    void bellRequested(const QString &message);
    void mouseUsageChanged();
    void scrolled(int line);

protected:
    bool event(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void inputMethodEvent(QInputMethodEvent *event) override;
    QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
    void timerEvent(QTimerEvent *event) override;
    void gestureEvent(QGestureEvent *event);

private:
    // ---- 绘制 ----
    void drawContent(QPainter &painter, const QRect &rect);
    void drawTextFragment(QPainter &painter, const QRect &rect,
                          const QString &text, const DTerminalChar &style);
    void drawBackground(QPainter &painter, const QRect &rect,
                        const QColor &color, bool useOpacity);
    void drawCursor(QPainter &painter, const QRect &rect,
                    const QColor &foreground, const QColor &background, bool &invertColors);
    void drawCharacters(QPainter &painter, const QRect &rect, const QString &text,
                        const DTerminalChar &style, bool invertCharacterColor);
    void drawLineCharString(QPainter &painter, int x, int y, const QString &text,
                            const DTerminalChar &style);
    void drawPreeditString(QPainter &painter, const QRect &rect);
    void drawLineChar(QPainter &painter, int x, int y, int w, int h, uint codePoint, bool bold);
    bool isLineCharString(const QString &text) const;
    QRect imageToWidget(const QRect &imageArea) const;
    QRect preeditRect() const;
    void paintScannerSpots(QPainter &painter);

    // ---- 几何与图像 ----
    void calcGeometry();
    void propagateSize();
    void updateImageSize();
    void makeImage();
    void scrollImage(int lines, const QRect &region);
    QPoint cursorCell() const;

    // ---- 交互 ----
    void extendSelection(const QPoint &pos);
    void initSelectionState();
    void initKeyboardSelection();
    void doDrag();
    QChar classifyChar(QChar c) const;
    void mouseTripleClickEvent(QMouseEvent *event);
    bool handleShortcutOverride(QKeyEvent *event);
    void showResizeNotification();
    void blinkEvent();
    void blinkCursorEvent();
    void enableBell();
    void swapColorTable();

    // ---- 手势（deepin 自有移植） ----
    void handleTapGesture(QTapGesture *gesture);
    void handleTapAndHoldGesture(QTapAndHoldGesture *gesture);
    void handlePanGesture(QPanGesture *gesture);
    void handlePinchGesture(QPinchGesture *gesture);
    void handleSwipeGesture(QSwipeGesture *gesture);
    void slideGesture(qreal diff);

    enum GestureAction {
        GestureNone,
        GestureTap,
        GestureSlide,
        GesturePinch,
        GestureHold,
        GesturePan,
        GestureSwipe
    };

    // ---- 成员 ----
    DTerminalScreen *m_screen = nullptr;
    DTerminalTextScanner *m_scanner = nullptr;

    DTerminalPaletteEntry m_palette[20];
    uint m_randomSeed = 0;

    bool m_fixedFont = false;
    int m_fontHeight = 1;
    int m_fontWidth = 1;
    int m_fontAscent = 0;
    bool m_boldIntense = true;
    int m_drawTextAdditionHeight = 0;
    bool m_drawTextTestFlag = false;

    int m_leftMargin = 0;
    int m_topMargin = 0;

    int m_lines = 0;
    int m_columns = 0;
    int m_usedLines = 0;
    int m_usedColumns = 0;

    QRect m_contentRect;
    int m_contentHeight = 0;
    int m_contentWidth = 0;

    QVector<DTerminalChar> m_image;      // [lines*columns]
    QVector<quint8> m_lineFlags;

    bool m_resizing = false;
    bool m_terminalSizeHint = false;
    bool m_terminalSizeStartup = false;
    bool m_bidiEnabled = false;
    bool m_usesMouse = false;
    bool m_alternateScrolling = true;
    bool m_bracketedPasteMode = false;

    QPoint m_selectionInitial;      // 初始选择点
    QPoint m_selectionCurrent;      // 当前选择点
    QPoint m_tripleSelectionBegin;
    int m_selectionState = 0;       // 0 无选择
    bool m_wordSelectionMode = false;
    bool m_lineSelectionMode = false;
    bool m_preserveLineBreaks = true;
    bool m_columnSelectionMode = false;

    QScrollBar *m_scrollBar = nullptr;
    ScrollBarPosition m_scrollbarLocation = ScrollBarRight;
    QString m_wordCharacters;
    int m_bellMode = 0;

    bool m_blinking = false;
    bool m_hasBlinker = false;
    bool m_cursorBlinking = false;
    bool m_cursorPositionSetEnable = false;
    bool m_hasBlinkingCursor = false;
    bool m_allowBlinkingText = true;
    bool m_ctrlDrag = false;
    TripleClickMode m_tripleClickMode = SelectWholeLine;
    bool m_isFixedSize = false;
    QTimer *m_blinkTimer = nullptr;
    QTimer *m_blinkCursorTimer = nullptr;

    bool m_dragPending = false;
    QPoint m_dragStart;
    bool m_possibleTripleClick = false;
    bool m_userIsResizing = false;
    QLabel *m_resizeWidget = nullptr;
    QTimer *m_resizeTimer = nullptr;

    bool m_flowControlWarningEnabled = false;
    bool m_hideCursor = false;
    QLabel *m_outputSuspendedLabel = nullptr;

    uint m_lineSpacing = 0;

    bool m_colorsInverted = false;
    QSize m_size;

    QRgb m_blendColor = qRgb(0, 0, 0);
    QPixmap m_backgroundImage;

    QRegion m_mouseOverSpotArea;

    DCursorShape m_cursorShape = DCursorShape::Block;
    QColor m_cursorColor;
    int m_motionAfterPasting = 0;

    struct PreeditData {
        QString preeditString;
        QRect previousPreeditRect;
    };
    PreeditData m_preedit;

    static bool s_antialiasText;
    static const int kTextBlinkDelay = 500;

    int m_margin = 0;
    bool m_centerContents = false;
    int m_leftBaseMargin = 0;
    int m_topBaseMargin = 0;

    int m_sessionId = 0;
    bool m_drawLineChars = true;

    // deepin：选区状态跟踪
    int m_selStartLine = 0;
    int m_selStartColumn = 0;
    int m_selEndLine = 0;
    int m_selEndColumn = 0;
    bool m_selBegin = false;

    bool m_isAllowScroll = true;

    // 滚动窗口状态（整合 ScreenWindow）
    int m_windowLines = 0;
    int m_currentLine = 0;
    bool m_trackOutput = true;
    int m_scrollCount = 0;

    // 手势状态（deepin 自有移植）
    qreal m_scaleFactor = 1.0;
    qreal m_currentStepScaleFactor = 1.0;
    qint64 m_tapBeginTime = 0;
    bool m_slideContinue = false;
    int m_lastMouseYpos = 0;
    ulong m_lastMouseTime = 0;
    qreal m_stepSpeed = 0;
    Qt::GestureState m_tapStatus = Qt::NoGesture;
    GestureAction m_gestureAction = GestureNone;
    QTimer *m_inertiaTimer = nullptr;
    qreal m_inertiaValue = 0;
};

#endif  // DTERMINALVIEW_H
