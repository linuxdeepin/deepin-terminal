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
    TermWidgetPage(TermProperties properties, QWidget * parent = nullptr);

    TermWidgetWrapper* currentTerminal();
    TermWidgetWrapper* split(Qt::Orientation orientation);
    TermWidgetWrapper* split(TermWidgetWrapper * term, Qt::Orientation orientation);
    void closeSplit(TermWidgetWrapper * term);
    const QString identifier();
    void focusCurrentTerm();
    void focusNavigation(NavigationDirection dir);

    TermProperties createCurrentTerminalProperties();

    void setColorScheme(const QString &name);

    void sendTextToCurrentTerm(const QString & text);

    void copyClipboard();
    void pasteClipboard();

    void zoomInCurrentTierminal();
    void zoomOutCurrentTerminal();

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
    TermWidgetWrapper * createTerm(TermProperties properties);

    TermWidgetWrapper * m_currentTerm;
};

#endif // TERMWIDGETPAGE_H
