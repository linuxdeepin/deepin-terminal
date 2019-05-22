#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include <qtermwidget.h>

class TermProperties;
class TermWidget : public QTermWidget
{
    Q_OBJECT
public:
    TermWidget(TermProperties properties, QWidget * parent = nullptr);

signals:
    void termRequestSplit(Qt::Orientation ori);
    void termRequestRenameTab(QString newTabName);

private slots:
    void customContextMenuCall(const QPoint & pos);
};

class TermWidgetWrapper : public QWidget
{
    Q_OBJECT
public:
    TermWidgetWrapper(TermProperties properties, QWidget * parent = nullptr);

    bool isTitleChanged() const;
    QString title() const;

    QString workingDirectory();

    void setColorScheme(const QString &name);

signals:
    void termRequestSplit(Qt::Orientation ori);
    void termRequestRenameTab(QString newTabName);
    void termTitleChanged(QString titleText);
    void termGetFocus();
    void termClosed();

private:
    void initUI();

    TermWidget * m_term;
    QVBoxLayout * m_layout;
};

#endif // TERMWIDGET_H
