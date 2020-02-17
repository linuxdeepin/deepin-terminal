#ifndef TERMWIDGETPAGE_H
#define TERMWIDGETPAGE_H

#include "define.h"
#include "termproperties.h"

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
    /********************* Modify by n014361 wangpeili End ************************/

public slots:
    void onTermRequestSplit(Qt::Orientation ori);
    void onTermRequestRenameTab(QString newTabName);
    void onTermTitleChanged(QString title) const;
    void onTermGetFocus();
    void onTermClosed();

signals:
    void tabTitleChanged(QString title) const;
    void termTitleChanged(QString title) const;
    void lastTermClosed(QString pageIdentifier) const;
    void termRequestOpenSettings() const;
    void termGetFocus() const;
    void termRequestOpenCustomCommand() const;
    void termRequestOpenRemoteManagement() const;

private slots:
    void setCurrentTerminal(TermWidgetWrapper *term);

private:
    TermWidgetWrapper *createTerm(TermProperties properties);

    TermWidgetWrapper *m_currentTerm = nullptr;
};
#endif  // TERMWIDGETPAGE_H
