#ifndef TERMWIDGETPAGE_H
#define TERMWIDGETPAGE_H

#include "define.h"
#include "termproperties.h"
#include "pagesearchbar.h"

#include <QWidget>

class TermWidgetWrapper;

class TermWidgetPage : public QWidget
{
    Q_OBJECT
public:
    TermWidgetPage(TermProperties properties, QWidget *parent = nullptr);

    TermWidgetWrapper *currentTerminal();
    TermWidgetWrapper *split(Qt::Orientation orientation);
    TermWidgetWrapper *split(TermWidgetWrapper *term, Qt::Orientation orientation);
    void closeSplit(TermWidgetWrapper *term);
    const QString identifier();
    void focusCurrentTerm();
    void closeOtherTerminal();
    void focusNavigation(Qt::Edge dir);
    int getTerminalCount();
    /******** Modify by n014361 wangpeili 2020-01-08: 计算上下左右判断方法 ******×****/
    QRect GetRect(TermWidgetWrapper *term);
    QPoint GetComparePoint(TermWidgetWrapper *term, Qt::Edge dir);
    /********************* Modify by n014361 wangpeili End ************************/

    TermProperties createCurrentTerminalProperties();

    void setTerminalOpacity(qreal opacity);
    void setColorScheme(const QString &name);

    void sendTextToCurrentTerm(const QString &text);

    void copyClipboard();
    void pasteClipboard();
    void toggleShowSearchBar();

    void zoomInCurrentTierminal();
    void zoomOutCurrentTerminal();

    /******** Modify by n014361 wangpeili 2020-01-06:增加相关设置功能 ***********×****/
    // 字体大小
    void setFontSize(int fontSize);
    // 字体
    void setFont(QString fontName);
    // 全选
    void selectAll();
    // 跳转到下一命令
    void skipToNextCommand();
    // 跳转到前一命令
    void skipToPreCommand();
    // 设置光标形状
    void setcursorShape(int shape);
    // 设置光标闪烁
    void setBlinkingCursor(bool enable);
    void setPressingScroll(bool enable);
    void showSearchBar(bool enable);
    /********************* Modify by n014361 wangpeili End ************************/
    void setTextCodec(QTextCodec *codec);
    void setMismatchAlert(bool alert);

protected:
    //将窗口设置为随着窗口变化而变化
    virtual void resizeEvent(QResizeEvent *event) override;

public slots:
    void onTermRequestSplit(Qt::Orientation ori);
    void onTermRequestRenameTab(QString newTabName);
    void onTermTitleChanged(QString title) const;
    void onTermGetFocus();
    void onTermClosed();
    // 查找下一个接口
    void handleFindNext();
    // 查找上一个接口
    void handleFindPrev();
    // 预留
    void slotFindbarClose();
    // 预留
    void handleRemoveSearchKeyword();
    // 更新搜索关键词接口
    void handleUpdateSearchKeyword(const QString &keyword);

signals:
    void pageRequestShowEncoding();
    void pageRequestNewWorkspace();
    void tabTitleChanged(QString title) const;
    void termTitleChanged(QString title) const;
    void lastTermClosed(QString pageIdentifier) const;
    void termRequestOpenSettings() const;
    void termGetFocus() const;
    void termRequestOpenCustomCommand() const;
    void termRequestOpenRemoteManagement() const;
    void termRequestUploadFile() const;
    void termRequestDownloadFile() const;

private slots:
    void setCurrentTerminal(TermWidgetWrapper *term);

private:
    TermWidgetWrapper *createTerm(TermProperties properties);

    TermWidgetWrapper *m_currentTerm = nullptr;
    PageSearchBar *m_findBar = nullptr;
};
#endif  // TERMWIDGETPAGE_H
