#ifndef TERMWIDGETPAGE_H
#define TERMWIDGETPAGE_H

#include <QWidget>

class TermWidgetWrapper;
class TermWidgetPage : public QWidget
{
    Q_OBJECT
public:
    TermWidgetPage(QWidget * parent = nullptr);

    TermWidgetWrapper* currentTerminal();

public slots:
    void onTermTitleChanged(QString title) const;

signals:
    void termTitleChanged(QString title) const;

private:
    TermWidgetWrapper * createTerm();

    TermWidgetWrapper * m_currentTerm;
};

#endif // TERMWIDGETPAGE_H
