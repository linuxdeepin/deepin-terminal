#include "customcommanditem.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAction>
#include <QDebug>
#include "customcommandoptdlg.h"
#include <DIconButton>

CustomCommandItem::CustomCommandItem(QAction *action, QWidget *parent) : DWidget(parent)
    , m_nameLabel(new DLabel(this))
    , m_shortcutLabel(new DLabel(this))
    , m_leftIconButton(new MyIconButton(this))
    , m_rightIconButton(new MyIconButton(this))
    , m_customCommandAction(action)
    , m_leftIcon(new DLabel(this))

{

    //setStyleSheet("border:2px sloid red;");
    //setStyleSheet("selection-color:rgb(241, 70, 62)");
    QImage img(":/images/icon/hover/circlebutton_add _hover.svg");
    m_leftIcon->setPixmap(QPixmap::fromImage(img));
    m_leftIcon->setFixedSize(QSize(70, 70));
    m_leftIcon->setStyleSheet("border:none;");
    m_leftIcon->setFocusPolicy(Qt::NoFocus);
    //m_customCommandAction = action;
    m_leftIconButton->setFixedSize(QSize(70, 70));
    m_leftIconButton->setStyleSheet("border:none;");
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
    m_rightIconButton->setStyleSheet("border:none;");
    //m_leftIconButton->setIcon(QIcon(":/images/icon/focus/circlebutton_add 2.svg"));
    m_rightIconButton->setIcon(QIcon(":/images/icon/hover/circlebutton_add _hover.svg"));
    if (m_customCommandAction) {
        QString strName = m_customCommandAction->text();
        QString strCommad = m_customCommandAction->data().toString();
//        QString strkey1 = m_customCommandAction->shortcut().toString();
//        QString strkey2 = m_customCommandAction->shortcut().toString(QKeySequence::NativeText);
//        qDebug() << "strkey1" << strkey1 << "strkey2" << strkey2 << endl;
        QKeySequence keyseq  = m_customCommandAction->shortcut();
        // QString strLabelTxt = strName + "\n" + keyseq.toString();
        m_nameLabel->setText(strName);
        m_shortcutLabel->setText(keyseq.toString());
    }
//    m_nameLabel->setFocusPolicy(Qt::NoFocus);
//    m_leftIcon->setFocusPolicy(Qt::NoFocus);
//    m_nameLabel->installEventFilter(this);
//    m_leftIcon->installEventFilter(this);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_leftIcon);
    //layout->addWidget(m_leftIconButton);
    layout->addLayout(vboxlayout);
    layout->addWidget(m_rightIconButton);
    layout->addStretch();
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    connect(m_rightIconButton, &DIconButton::clicked, this, &CustomCommandItem::editCustomCommand);
    // connect(m_leftIcon, &DIconButton::focusOutEvent, this, &CustomCommandItem::editCustomCommand);
    //setFixedSize(50, 250);
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
    QKeySequence keyseq  = action->shortcut();
    m_nameLabel->setText(strName);
    m_shortcutLabel->setText(keyseq.toString());
}

