#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include "terminalwidget/lib/qtermwidget.h"
#include "termwidgetpage.h"

class TermProperties;
class TermWidget : public QTermWidget
{
    Q_OBJECT
public:
    TermWidget(TermProperties properties, QWidget *parent = nullptr);
    TermWidgetPage * parentPage();
    bool isInRemoteServer();
public:
    // 跳转到下一个命令
    void skipToNextCommand();
    // 跳转到前一个命令
    void skipToPreCommand();
    void setTermOpacity(qreal opacity);
    // 修改字体
    void setTermFont(const QString &fontName);
    // 修改字体大小
    void setTermFontSize(const int fontSize);
    // 修改光标形状
    void setCursorShape(int shape);
    void setPressingScroll(bool enable);

public slots:
    void wpasteSelection();
    void onSettingValueChanged(const QString &keyName);

signals:
    void termRequestRenameTab(QString newTabName);
    void termIsIdle(int currSessionId, bool bIdle);
    void termTitleChanged(QString titleText);

private slots:
    void customContextMenuCall(const QPoint &pos);
    void handleTermIdle(bool bIdle);
private:
    TermWidgetPage *m_Page = nullptr;
    TermProperties m_properties;

public:
    // session支持最大值．再多了就起不来了．
    static const int MaxTermwidgetCount = 199;
};

#endif  // TERMWIDGET_H
