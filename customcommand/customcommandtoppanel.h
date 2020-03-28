#ifndef CUSTOMCOMMANDTOPPANEL_H
#define CUSTOMCOMMANDTOPPANEL_H

#include "rightpanel.h"
#include "customcommandpanel.h"
#include "customcommandsearchrstpanel.h"

#include <QWidget>

class CustomCommandTopPanel : public RightPanel
{
    Q_OBJECT
public:
    explicit CustomCommandTopPanel(QWidget *parent = nullptr);
    void show();

protected:
    void resizeEvent(QResizeEvent *event) override;

signals:
    void focusOut();
    void handleCustomCurCommand(const QString &strCommand);

public slots:
    void showCustomCommandPanel();
    void showCustomCommandSearchPanel(const QString &strFilter);

private:
    CustomCommandPanel *m_customCommandPanel = nullptr;
    CustomCommandSearchRstPanel *m_customCommandSearchPanel = nullptr;
};

#endif  // CUSTOMCOMMANDTOPPANEL_H
