#ifndef CUSTOMCOMMANDTOPPANEL_H
#define CUSTOMCOMMANDTOPPANEL_H

#include <QWidget>
#include "rightpanel.h"
#include "customcommandpanel.h"
#include "customcommandsearchrstpanel.h"
class CustomCommandTopPanel : public RightPanel
{
    Q_OBJECT
public:
    explicit CustomCommandTopPanel(QWidget *parent = nullptr);
    void show();

signals:
    void focusOut();
    void handleCustomCurCommand(const QString &strCommand);
public slots:
    void showCustomCommandPanel();
    void showCustomCommandSearchPanel(const QString &strFilter);
private:
    CustomCommandPanel *m_customCommandPanel;
    CustomCommandSearchRstPanel *m_customCommandSearchPanel;
};

#endif // CUSTOMCOMMANDTOPPANEL_H
