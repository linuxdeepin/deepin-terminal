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
    void termRequestSplit(Qt::Orientation ori);
    void termRequestRenameTab(QString newTabName);
    void termRequestOpenSettings();
    void termRequestOpenCustomCommand();
    void termRequestOpenRemoteManagement();
    void termIsIdle(int currSessionId, bool bIdle);
    void termRequestUploadFile();
    void termRequestDownloadFile();
    void termTitleChanged(QString titleText);

private slots:
    void customContextMenuCall(const QPoint &pos);
    void handleTermIdle(bool bIdle);
private:
    void *m_Page = nullptr;
};

#endif  // TERMWIDGET_H
