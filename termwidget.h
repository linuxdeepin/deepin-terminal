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

    bool isTitleChanged() const;
    QString title() const;

    QString workingDirectory();

    void sendText(const QString &text);
    void setTerminalOpacity(qreal opacity);
    void setColorScheme(const QString &name);

public slots:
    void zoomIn();
    void zoomOut();
    void copyClipboard();
    void pasteClipboard();

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

    TermWidget *m_term;
    QVBoxLayout *m_layout;
};

#endif // TERMWIDGET_H
