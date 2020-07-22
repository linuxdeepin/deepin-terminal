#include "termcommandlinkbutton.h"

#include <DApplicationHelper>

TermCommandLinkButton::TermCommandLinkButton(QWidget *parent)
    : DPushButton(parent)
{
    DPalette palette = DApplicationHelper::instance()->palette(this);
    palette.setColor(DPalette::ButtonText, palette.color(DPalette::TextWarning));
    this->setPalette(palette);

    this->setFlat(true);
    this->setFocusPolicy(Qt::TabFocus);
}
