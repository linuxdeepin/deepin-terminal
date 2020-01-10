#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include <qtermwidget5/qtermwidget.h>

class TermProperties;
class TermWidget : public QTermWidget
{
    Q_OBJECT
public:
    TermWidget(TermProperties properties, QWidget *parent = nullptr);

signals:
    void termRequestSplit(Qt::Orientation ori);
    void termRequestRenameTab(QString newTabName);
    void termRequestOpenSettings();
    void termRequestOpenCustomCommand();
private slots:
    void customContextMenuCall(const QPoint &pos);
};

class TermWidgetWrapper : public QWidget
{
    Q_OBJECT
public:
    TermWidgetWrapper(TermProperties properties, QWidget *parent = nullptr);

    bool    isTitleChanged() const;
    QString title() const;

    QString workingDirectory();

    void sendText(const QString &text);
    void setTerminalOpacity(qreal opacity);
    void setColorScheme(const QString &name);
    /******** Modify by n014361 wangpeili 2020-01-06: 修改字体      ***********×****/
    // 修改字体
    void setTerminalFont(const QString &fontName);
    // 修改字体大小
    void setTerminalFontSize(const int fontSize);
    // 全选
    void selectAll();
    // 跳转到下一个命令
    void skipToNextCommand();
    // 跳转到前一个命令
    void skipToPreCommand();
    // 修改光标形状
    void setCursorShape(int shape);
    // 设置光标是否闪烁
    void setCursorBlinking(bool enable);
    /********************* Modify by n014361 wangpeili End ************************/

public slots:
    void zoomIn();
    void zoomOut();
    void copyClipboard();
    void pasteClipboard();
    /******** Modify by n014361 wangpeili 2020-01-10: 粘贴选择内容   ****************/
    void pasteSelection();
    /********************* Modify by n014361 wangpeili End ************************/

signals:
    void termRequestSplit(Qt::Orientation ori);
    void termRequestRenameTab(QString newTabName);
    void termTitleChanged(QString titleText);
    void termRequestOpenSettings();
    void termGetFocus();
    void termClosed();
    void termRequestOpenCustomCommand();

private:
    void initUI();

    TermWidget * m_term;
    QVBoxLayout *m_layout;
};

#endif  // TERMWIDGET_H
