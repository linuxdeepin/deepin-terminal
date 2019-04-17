#ifndef TERMWIDGET_H
#define TERMWIDGET_H

#include <qtermwidget.h>

class TermWidget : public QTermWidget
{
    Q_OBJECT
public:
    TermWidget(QWidget * parent = nullptr);

private slots:
    void customContextMenuCall(const QPoint & pos);
};

class TermWidgetWrapper : public QWidget
{
    Q_OBJECT
public:
    TermWidgetWrapper(QWidget * parent = nullptr);

signals:
    void termTitleChanged(QString titleText);

private:
    void initUI();

    TermWidget * m_term;
    QVBoxLayout * m_layout;
};

#endif // TERMWIDGET_H
