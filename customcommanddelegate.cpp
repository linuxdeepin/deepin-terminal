#include "customcommanddelegate.h"
#include "customcommanditemmodel.h"
#include "utils.h"

#include <DGuiApplicationHelper>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyleHelper>

#include <QPainter>
#include <QPixmap>
#include <DLog>

CustomCommandDelegate::CustomCommandDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_parentView(parent)
{
}

void CustomCommandDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QVariant varDisplay = index.data(Qt::DisplayRole);

        CustomCommandItemData itemData = varDisplay.value<CustomCommandItemData>();

        QStyleOptionViewItem viewOption(option);  //用来在视图中画一个item

        DPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? DPalette::Normal : DPalette::Disabled;
        if (cg == DPalette::Normal && !(option.state & QStyle::State_Active)) {
            cg = DPalette::Inactive;
        }

        QRect bgRect;
        bgRect.setX(option.rect.x() + 10);
        bgRect.setY(option.rect.y());
        bgRect.setWidth(option.rect.width() - 10);
        bgRect.setHeight(option.rect.height());

        int leftIconSize = 70;
        int rightIconSize = 30;

        QString strLeftIconSrc = ":/images/icon/hover/circlebutton_add _hover.svg";
        QPixmap leftIconPixmap = Utils::renderSVG(strLeftIconSrc, QSize(leftIconSize, leftIconSize));

        QRect leftIconRect = QRect(bgRect.left(), bgRect.top() + (bgRect.height() - leftIconSize) / 2,
                                   leftIconSize, leftIconSize);
        painter->drawPixmap(leftIconRect, leftIconPixmap);


        QString strRightIconSrc = ":/images/icon/hover/edit_hover.svg";
        QPixmap rightIconPixmap = Utils::renderSVG(strRightIconSrc, QSize(rightIconSize, rightIconSize));
        QRect rightIconRect = QRect(bgRect.right() - rightIconSize - 10, bgRect.top() + (bgRect.height() - rightIconSize) / 2,
                                    rightIconSize, rightIconSize);
        painter->drawPixmap(rightIconRect, rightIconPixmap);

        QString strCmdName = "";
        QString strCmdShortcut = "";
        QAction *customCommandAction = itemData.m_customCommandAction;
        if (customCommandAction) {
            QString strCommad = customCommandAction->data().toString();
            QKeySequence keyseq = customCommandAction->shortcut();
            strCmdName = customCommandAction->text();
            strCmdShortcut = keyseq.toString();
        }

        int labelHeight = 35;
        QRect cmdNameRect = QRect(bgRect.left() + leftIconSize + 5, bgRect.top() + 5, bgRect.width() - leftIconSize - rightIconSize, labelHeight);
        painter->drawText(cmdNameRect, Qt::AlignLeft | Qt::AlignVCenter, strCmdName);

        QRect cmdShortcutRect = QRect(bgRect.left() + leftIconSize + 5, bgRect.bottom() - labelHeight - 5, bgRect.width() - leftIconSize - rightIconSize, labelHeight);
        painter->drawText(cmdShortcutRect, Qt::AlignLeft | Qt::AlignVCenter, strCmdShortcut);

        painter->restore();
    } else {
        DStyledItemDelegate::paint(painter, option, index);
    }
}

QSize CustomCommandDelegate::sizeHint(const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    Q_UNUSED(index)

    return QSize(option.rect.width(), 70);
}
