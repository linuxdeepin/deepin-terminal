#include "termbasedialog.h"

#include <QLabel>
#include <DApplicationHelper>

static void palrtteTransparency(QWidget *widget, qint8 alphaFloat)
{
    QPalette palette = widget->palette();
    QColor color = DGuiApplicationHelper::adjustColor(palette.color(QPalette::BrightText), 0, 0, 0, 0, 0, 0, alphaFloat);
    palette.setColor(QPalette::WindowText, color);
    widget->setPalette(palette);
}

//fix bug 23481 主菜单切换主题，弹框字体颜色没有随主题及时变换
void QWidget::paintEvent(QPaintEvent *e)
{
    if (strcmp(this->metaObject()->className(), "Dtk::Widget::DDialog") != 0)
    {
        return;
    }

    QLabel *titleLabel = this->findChild<QLabel *>("TitleLabel");
    QLabel *messageLabel = this->findChild<QLabel *>("MessageLabel");

    if (titleLabel)
    {
        palrtteTransparency(titleLabel, -10);
    }

    if (messageLabel)
    {
        palrtteTransparency(messageLabel, -30);
    }
}
