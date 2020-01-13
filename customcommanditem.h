#ifndef CUSTOMCOMMANDITEM_H
#define CUSTOMCOMMANDITEM_H

#include <QWidget>
#include <DWidget>
#include <DIconButton>
#include <DLabel>
#include <DIconButton>
#include <QAction>
#include "myiconbutton.h"
DWIDGET_USE_NAMESPACE
class CustomCommandItem : public DWidget
{
    Q_OBJECT
public:
    explicit CustomCommandItem(QAction *action, QWidget *parent = nullptr);
    QAction *getCurCustomCommandAction();
    void refreshCommandInfo(QAction *action);
signals:
    //void modifyCustomCommand(QAction *action);
    void modifyCustomCommand(CustomCommandItem *item);

private slots:
    void editCustomCommand();
protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
private:
    DLabel *m_nameLabel;
    DLabel *m_shortcutLabel;
    MyIconButton *m_leftIconButton;
    MyIconButton *m_rightIconButton;
    QAction *m_customCommandAction;
    DLabel *m_leftIcon;
};

#endif // CUSTOMCOMMANDITEM_H
