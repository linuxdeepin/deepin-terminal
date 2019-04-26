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
    void onTermRequestSplit(Qt::Orientation ori);
    void onTermRequestRenameTab(QString newTabName);
    void onTermTitleChanged(QString title) const;
    void onTermGetFocus();
    void onTermClosed();

signals:
    void tabTitleChanged(QString title) const;
    void termTitleChanged(QString title) const;
    void lastTermClosed(QString pageIdentifier) const;

private slots:
    void setCurrentTerminal(TermWidgetWrapper * term);

private:
    TermWidgetWrapper * createTerm();

    TermWidgetWrapper * m_currentTerm;
};

#endif // TERMWIDGETPAGE_H
