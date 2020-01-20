#include "customcommanditem.h"
#include "customcommandoptdlg.h"

#include <DIconButton>

#include <QAction>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>

CustomCommandItem::CustomCommandItem(QAction *action, QWidget *parent)
    : DWidget(parent),
      m_nameLabel(new DLabel(this)),
      m_shortcutLabel(new DLabel(this)),
      m_leftIconButton(new MyIconButton(this)),
      m_rightIconButton(new MyIconButton(this)),
      m_customCommandAction(action),
      m_leftIcon(new DLabel(this))

{
    QImage img(":/images/icon/hover/circlebutton_add _hover.svg");
    m_leftIcon->setPixmap(QPixmap::fromImage(img));
    m_leftIcon->setFixedSize(QSize(70, 70));

    m_leftIcon->setFocusPolicy(Qt::NoFocus);
    m_leftIconButton->setFixedSize(QSize(70, 70));

    m_leftIconButton->setIcon(QStyle::StandardPixmap::SP_DialogYesButton);

    m_nameLabel->setFixedSize(QSize(110, 35));
    m_nameLabel->setAlignment(Qt::AlignLeft);
    m_shortcutLabel->setFixedSize(QSize(110, 35));
    m_shortcutLabel->setAlignment(Qt::AlignLeft);
    QVBoxLayout *vboxlayout = new QVBoxLayout();
    vboxlayout->addWidget(m_nameLabel);
    vboxlayout->addWidget(m_shortcutLabel);
    vboxlayout->setMargin(0);
    vboxlayout->setSpacing(0);

    m_rightIconButton->setFixedSize(QSize(70, 70));

    m_rightIconButton->setIcon(QIcon(":/images/icon/hover/circlebutton_add _hover.svg"));
    if (m_customCommandAction) {
        QString strName = m_customCommandAction->text();
        QString strCommad = m_customCommandAction->data().toString();
        QKeySequence keyseq = m_customCommandAction->shortcut();
        m_nameLabel->setText(strName);
        m_shortcutLabel->setText(keyseq.toString());
    }

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_leftIcon);
    layout->addLayout(vboxlayout);
    layout->addWidget(m_rightIconButton);
    layout->addStretch();
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    connect(m_rightIconButton, &DIconButton::clicked, this, &CustomCommandItem::editCustomCommand);
    m_rightIconButton->hide();
}

QAction *CustomCommandItem::getCurCustomCommandAction()
{
    return m_customCommandAction;
}

void CustomCommandItem::editCustomCommand()
{
    emit modifyCustomCommand(this);
}

void CustomCommandItem::enterEvent(QEvent *event)
{
    m_rightIconButton->show();
    DWidget::enterEvent(event);
}

void CustomCommandItem::leaveEvent(QEvent *event)
{
    m_rightIconButton->hide();
    DWidget::enterEvent(event);
}

void CustomCommandItem::refreshCommandInfo(QAction *action)
{
    QString strName = action->text();
    QString strCommad = action->data().toString();
    QKeySequence keyseq = action->shortcut();
    m_nameLabel->setText(strName);
    m_shortcutLabel->setText(keyseq.toString());
}
