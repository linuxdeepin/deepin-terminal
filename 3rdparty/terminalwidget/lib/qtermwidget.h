/*  Copyright (C) 2008 e_k (e_k@users.sourceforge.net)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#ifndef _Q_TERM_WIDGET
#define _Q_TERM_WIDGET

#include <QTranslator>
#include <QWidget>
#include <QPointer>
#include "Emulation.h"
#include "Filter.h"
#include "HistorySearch.h"

#include "qtermwidget_export.h"

class QVBoxLayout;
class TermWidgetImpl;
class SearchBar;
class QUrl;
namespace Konsole {
class TerminalDisplay;
}

class TERMINALWIDGET_EXPORT QTermWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * This enum describes the location where the scroll bar is positioned in the display widget.
     */
    enum ScrollBarPosition {
        /** Do not show the scroll bar. */
        NoScrollBar = 0,
        /** Show the scroll bar on the left side of the display. */
        ScrollBarLeft = 1,
        /** Show the scroll bar on the right side of the display. */
        ScrollBarRight = 2
    };

    using KeyboardCursorShape = Konsole::Emulation::KeyboardCursorShape;

    //Creation of widget
    QTermWidget(int startnow, // 1 = start shell programm immediatelly
                QWidget *parent = nullptr);
    // A dummy constructor for Qt Designer. startnow is 1 by default
    explicit QTermWidget(QWidget *parent = nullptr);

    ~QTermWidget() override;

    //Initial size
    QSize sizeHint() const override;

    // expose TerminalDisplay::TerminalSizeHint, setTerminalSizeHint
    void setTerminalSizeHint(bool on);
    bool terminalSizeHint();

    //start shell program if it was not started in constructor
    void startShellProgram();

    // Returns session id list of processes running in the terminal window
    QList<int> getRunningSessionIdList();

    // Determine whether the terminal control has running process
    bool hasRunningProcess();

    /**
     * Start terminal teletype as is
     * and redirect data for external recipient.
     * It can be used for display and control a remote terminal.
     */
    void startTerminalTeletype();

    int getShellPID();

    // Get current terminal's session id
    int getSessionId();

    void changeDir(const QString &dir);

    // look-n-feel, if you don`t like defaults

    //  Terminal font
    // Default is application font with family Monospace, size 10
    // Beware of a performance penalty and display/alignment issues when using a proportional font.
    void setTerminalFont(const QFont &font);
    QFont getTerminalFont();
    void setTerminalOpacity(qreal level);
    void setTerminalBackgroundImage(QString backgroundImage);

    // environment
    void setEnvironment(const QStringList &environment);

    //  Shell program, default is /bin/bash
    void setShellProgram(const QString &progname);

    // working directory
    void setWorkingDirectory(const QString &dir);
    QString workingDirectory();

    // Shell program args, default is none
    void setArgs(const QStringList &args);

    // Text codec, default is UTF-8
    void setTextCodec(QTextCodec *codec);

    void setTerminalWordCharacters(const QString &wc);

    /** @brief Sets the color scheme, default is white on black
     *
     * @param[in] name The name of the color scheme, either returned from
     * availableColorSchemes() or a full path to a color scheme.
     */
    //设置主题的配色方案
    int setColorScheme(const QString &name);
    static QStringList availableColorSchemes();
    static void addCustomColorSchemeDir(const QString &custom_dir);

    // History size for scrolling
    void setHistorySize(int lines);  // infinite if lines < 0

    // Presence of scrollbar
    void setScrollBarPosition(ScrollBarPosition);

    // Wrapped, scroll to end.
    void scrollToEnd();

    /******** Modify by n014361 wangpeili 2020-02-17:设置当前屏幕是否滚动****************/
    void setTrackOutput(bool enable);
    /********************* Modify by n014361 wangpeili End ************************/

    // Send some text to terminal
    void sendText(const QString &text);

    // Send key event to terminal
    void sendKeyEvent(QKeyEvent *e);

    // Sets whether flow control is enabled
    void setFlowControlEnabled(bool enabled);

    // Returns whether flow control is enabled
    bool flowControlEnabled(void);

    /**
     * Sets whether the flow control warning box should be shown
     * when the flow control stop key (Ctrl+S) is pressed.
     */
    void setFlowControlWarningEnabled(bool enabled);

    /*! Get all available keyboard bindings
     */
    static QStringList availableKeyBindings();

    //! Return current key bindings
    QString keyBindings();

    void setMotionAfterPasting(int);

    /** Return the number of lines in the history buffer. */
    int historyLinesCount();

    int screenColumnsCount();
    int screenLinesCount();

    void setSelectionStart(int row, int column);
    void setSelectionEnd(int row, int column);
    /******** Modify by n014361 wangpeili 2020-02-13: 新增屏幕全选功能***********×****/
    void setSelectionAll();
    /***************** Modify by n014361 End *************************/
    void getSelectionStart(int &row, int &column);
    void getSelectionEnd(int &row, int &column);
    /******** Modify by ut000610 daizhengwen 2020-06-02: 获得pty erase模式****************/
    char getErase();
    /********************* Modify by ut000610 daizhengwen End ************************/

    // 设置删除键模式（修改内存接口）
    void setDeleteMode(char *key, int length);
    // 设置退格按键模式（用户选择接口）
    void setBackspaceMode(char *key, int length);

    /**
     * Returns the currently selected text.
     * @param preserveLineBreaks Specifies whether new line characters should
     * be inserted into the returned text at the end of each terminal line.
     */
    QString selectedText(const Screen::DecodingOptions options = Screen::PreserveLineBreaks);

    void setMonitorActivity(bool);
    void setMonitorSilence(bool);
    void setSilenceTimeout(int seconds);

    /** Returns the available hotspot for the given point \em pos.
     *
     * This method may return a nullptr if no hotspot is available.
     *
     * @param[in] pos The point of interest in the QTermWidget coordinates.
     * @return Hotspot for the given position, or nullptr if no hotspot.
     */
    Filter::HotSpot *getHotSpotAt(const QPoint &pos) const;

    /** Returns the available hotspots for the given row and column.
     *
     * @return Hotspot for the given position, or nullptr if no hotspot.
     */
    Filter::HotSpot *getHotSpotAt(int row, int column) const;

    /*
     * Proxy for TerminalDisplay::filterActions
     * */
    QList<QAction *> filterActions(const QPoint &position);

    /**
     * Returns a pty slave file descriptor.
     * This can be used for display and control
     * a remote terminal.
     */
    int getPtySlaveFd() const;

    /**
     * Sets the shape of the keyboard cursor.  This is the cursor drawn
     * at the position in the terminal where keyboard input will appear.
     */
    void setKeyboardCursorShape(KeyboardCursorShape shape);

    void setBlinkingCursor(bool blink);

    /** Enables or disables bidi text in the terminal. */
    void setBidiEnabled(bool enabled);
    bool isBidiEnabled();

    /**
     * Automatically close the terminal session after the shell process exits or
     * keep it running.
     */
    void setAutoClose(bool);

    QString title() const;
    QString icon() const;

    /** True if the title() or icon() was (ever) changed by the session. */
    bool isTitleChanged() const;

    /** change and wrap text corresponding to paste mode **/
    void bracketText(QString &text);

    /** Set the empty space outside the terminal */
    void setMargin(int);

    /** Get the empty space outside the terminal */
    int getMargin() const;

    /** Get the foreground pid in terminal */
    int getForegroundProcessId() const;

    // 获取foreground名称
    QString getForegroundProcessName() const;

    void setDrawLineChars(bool drawLineChars);

    void setBoldIntense(bool boldIntense);

    // 获取是否允许输出时滚动
    bool getIsAllowScroll() const;

    // 设置是否允许输出时滚动
    void setIsAllowScroll(bool isAllowScroll);

    //Add by ut001000 renfeixiang 2020-12-02 当搜索框出现时，设置m_bHasSelect为false,
    //避免搜索框隐藏再显示之后，继续走m_bHasSelect为true流程，导致崩溃
    void setNoHasSelect();

signals:
    void finished();
    void copyAvailable(bool);

    void termGetFocus();
    void termLostFocus();
    void leftMouseClick();

    void termKeyPressed(QKeyEvent *);

    void urlActivated(const QUrl &, bool fromContextMenu);

    void bell(const QString &message);

    void activity();
    void silence();

    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/
    bool uninstallTerminal(QString commandname);
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/

    /******** Modify by ut000610 daizhengwen 2020-06-11: QProcess start finished signal****************/
    void processStarted();
    /********************* Modify by ut000610 daizhengwen End ************************/

    /******** Modify by ut000610 daizhengwen 2020-09-03:将拖拽过来的数据传送给终端 Begin***************/
    void sendUrlsToTerm(const char *);
    /********************* Modify by ut000610 daizhengwen End ************************/
    /**
     * Emitted when emulator send data to the terminal process
     * (redirected for external recipient). It can be used for
     * control and display the remote terminal.
     */
    void sendData(const char *, int, const QTextCodec *);

    void profileChanged(const QString &profile);

    void titleChanged();

    // warning提示信息 currentShell当前使用的shell, 启用shell是否成功 true 替换了shell false 替换shell但启动失败
    void shellWarningMessage(QString currentShell, bool isSuccess);

    /**
     * Signals that we received new data from the process running in the
     * terminal emulator
     */
    void receivedData(const QString &text);

    /**
     * Signals for dynamically determine whether current terminal is busy or idle
     */
    void isTermIdle(bool bIdle);
    // 将库里返回信号透传出来。原来的noMatchFound方法改名为clearSelection
    void sig_noMatchFound();
    // 找到的信号
    void sig_matchFound();

    // 标签标题参数改变 dzw 2020-12-2
    void titleArgsChange(QString key, QString value);

public slots:
    // Copy selection to clipboard
    void copyClipboard();

    // Paste clipboard to terminal
    void pasteClipboard();

    // Paste selection to terminal
    void pasteSelection();

    // Set zoom
    void zoomIn();
    void zoomOut();

    // Set size
    void setSize(const QSize &);

    /*! Set named key binding for given widget
     */
    void setKeyBindings(const QString &kb);

    /*! Clear the terminal content and move to home position
     */
    void clear();

    void toggleShowSearchBar();
    /******** Modify by n014361 wangpeili 2020-02-24:              ****************/
    // 新增外部搜索接口，不用qterminal内置的搜索接口
    void search(QString txt, bool forwards, bool next);
    // 清除选中框
    void clearSelection();

    void noMatchFound();
    /********************* Modify by n014361 wangpeili End ************************/

    void saveHistory(QIODevice *device);
protected:
    void resizeEvent(QResizeEvent *) override;

protected slots:
    void sessionFinished();
    void selectionChanged(bool textSelected);

private slots:
    void matchFound(int startColumn, int startLine, int endColumn, int endLine, int lastBackwardsPosition, int loseChinese, int matchChinese);

    /**
     * Emulation::cursorChanged() signal propogates to here and QTermWidget
     * sends the specified cursor states to the terminal display
     */
    void cursorChanged(Konsole::Emulation::KeyboardCursorShape cursorShape, bool blinkingCursorEnabled);
    void snapshot();

private:
    void setZoom(int step);
    void init(int startnow);
    void addSnapShotTimer();
    void interactionHandler();

    TermWidgetImpl *m_impl;
    SearchBar *m_searchBar;
    QVBoxLayout *m_layout;
    static QTranslator *m_translator;
    QPointer<Konsole::TerminalDisplay> m_termDisplay;
    QTimer *m_interactionTimer = nullptr;

    bool m_bHasSelect = false;
    int m_startColumn = 0;
    int m_startLine = 0;
    int m_endColumn = 0;
    int m_endLine = 0;
    int m_lastBackwardsPosition = -1;
    //上一次是正向搜索还是反向搜索
    bool m_isLastForwards = false;
};

// Maybe useful, maybe not

#ifdef __cplusplus
extern "C"
#endif
void *createTermWidget(int startnow, void *parent);

#endif

