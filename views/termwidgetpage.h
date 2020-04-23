#ifndef TERMWIDGETPAGE_H
#define TERMWIDGETPAGE_H

#include "define.h"
#include "termproperties.h"
#include "pagesearchbar.h"
#include "mainwindow.h"

#include <DSplitter>

#include <QWidget>

DWIDGET_USE_NAMESPACE

class TermWidget;
class MainWindow;
class TermWidgetPage : public QWidget
{
    Q_OBJECT
public:
    TermWidgetPage(TermProperties properties, QWidget *parent = nullptr);
    // mainwindow指针，parent()会变化？？？所以要在构造的时候保存。
    MainWindow *parentMainWindow();
    TermWidget *currentTerminal();
    TermWidget *split(Qt::Orientation orientation);
    TermWidget *split(TermWidget *term, Qt::Orientation orientation);
    void closeSplit(TermWidget *term);
    // 标识page的唯一ID， 和tab匹配，存在tab中，tabid 用的index，是变化的。
    const QString identifier();
    void focusCurrentTerm();
    void closeOtherTerminal();
    void focusNavigation(Qt::Edge dir);
    int getTerminalCount();
    /******** Modify by n014361 wangpeili 2020-01-08: 计算上下左右判断方法 ******×****/
    QRect GetRect(TermWidget *term);
    QPoint GetComparePoint(TermWidget *term, Qt::Edge dir);
    /********************* Modify by n014361 wangpeili End ************************/
    bool hasRunningProcess();
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
    /******** Modify by n014361 wangpeili 2020-03-11: 非DTK控件手动匹配系统主题的修改 **********/
    void applyTheme();
    /********************* Modify by n014361 wangpeili End ************************/
    void updateSplitStyle();

signals:
    void tabTitleChanged(QString title) const;
    void termTitleChanged(QString title) const;
    void lastTermClosed(QString pageIdentifier) const;
    void termRequestOpenSettings() const;
    void termGetFocus() const;
    bool uninstallTerminal();

private slots:
    void setCurrentTerminal(TermWidget *term);

private:
    TermWidget *createTerm(TermProperties properties);
    void setSplitStyle(DSplitter *splitter);

    TermWidget *m_currentTerm = nullptr;
    PageSearchBar *m_findBar = nullptr;
    MainWindow *m_MainWindow = nullptr;
};
#endif  // TERMWIDGETPAGE_H
