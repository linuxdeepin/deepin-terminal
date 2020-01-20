#ifndef CUSTOMCOMMANDITEM_H
#define CUSTOMCOMMANDITEM_H

#include "myiconbutton.h"

#include <DWidget>
#include <DIconButton>
#include <DLabel>
#include <DIconButton>

#include <QWidget>
#include <QAction>

DWIDGET_USE_NAMESPACE

class CustomCommandItem : public DWidget
{
    Q_OBJECT
public:
    explicit CustomCommandItem(QAction *action, QWidget *parent = nullptr);
    QAction *getCurCustomCommandAction();
    void refreshCommandInfo(QAction *action);

signals:
    void modifyCustomCommand(CustomCommandItem *item);

private slots:
    void editCustomCommand();

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    DLabel *m_nameLabel = nullptr;
    DLabel *m_shortcutLabel = nullptr;
    MyIconButton *m_leftIconButton = nullptr;
    MyIconButton *m_rightIconButton = nullptr;
    QAction *m_customCommandAction = nullptr;
    DLabel *m_leftIcon = nullptr;
};

#endif  // CUSTOMCOMMANDITEM_H
