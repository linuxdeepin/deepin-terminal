#ifndef TERMCOMMANDLINKBUTTON_H
#define TERMCOMMANDLINKBUTTON_H

#include <DPushButton>

DWIDGET_USE_NAMESPACE

class TermCommandLinkButton : public DPushButton
{
    Q_OBJECT
public:
    explicit TermCommandLinkButton(QWidget *parent = nullptr);
};

#endif // TERMCOMMANDLINKBUTTON_H
