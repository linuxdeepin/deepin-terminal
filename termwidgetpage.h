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
    TermWidgetWrapper* split(TermWidgetWrapper * term, Qt::Orientation orientation);
    void closeSplit(TermWidgetWrapper * term);
    const QString identifier();

public slots:
    void onTermTitleChanged(QString title) const;
    void onTermClosed();

signals:
    void termTitleChanged(QString title) const;
    void lastTermClosed(QString pageIdentifier) const;

private:
    TermWidgetWrapper * createTerm();

    TermWidgetWrapper * m_currentTerm;
};

#endif // TERMWIDGETPAGE_H
