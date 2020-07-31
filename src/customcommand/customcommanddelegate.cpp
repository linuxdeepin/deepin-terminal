#include "customcommanddelegate.h"
#include "customcommanditemmodel.h"
#include "utils.h"

#include <DGuiApplicationHelper>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyleHelper>

#include <QPainter>
#include <QPainterPath>
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
        //对有效的索引进行重绘处理
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
        bgRect.setY(option.rect.y() + 10);
        bgRect.setWidth(option.rect.width() - 20);
        bgRect.setHeight(option.rect.height() - 10);

        QPainterPath path;
        int cornerSize = 16;
        int arcRadius = 8;

        path.moveTo(bgRect.left() + arcRadius, bgRect.top());
        path.arcTo(bgRect.left(), bgRect.top(), cornerSize, cornerSize, 90.0, 90.0);
        path.lineTo(bgRect.left(), bgRect.bottom() - arcRadius);
        path.arcTo(bgRect.left(), bgRect.bottom() - cornerSize, cornerSize, cornerSize, 180.0, 90.0);
        path.lineTo(bgRect.right() - arcRadius, bgRect.bottom());
        path.arcTo(bgRect.right() - cornerSize, bgRect.bottom() - cornerSize, cornerSize, cornerSize, 270.0, 90.0);
        path.lineTo(bgRect.right(), bgRect.top() + arcRadius);
        path.arcTo(bgRect.right() - cornerSize, bgRect.top(), cornerSize, cornerSize, 0.0, 90.0);
        path.lineTo(bgRect.left() + arcRadius, bgRect.top());

        if (option.state & QStyle::State_MouseOver) {
            //鼠标悬浮在item上的状态，填充绘制背景
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::ToolTipText);
            fillColor.setAlphaF(0.1);//fillColor.setAlphaF(0.3);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, fillColor);
        } else {
            //鼠标未悬浮在item上的状态，填充绘制背景
            DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ItemBackground);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, fillColor);
        }

        int cmdIconSize = 44;
        int editIconSize = 20;
        QRect modifyRect = QRect(bgRect.right() - editIconSize - 6, bgRect.top() + (bgRect.height() - editIconSize) / 2,
                                 editIconSize, editIconSize);

        QRect cmdIconRect = QRect(bgRect.left() + 6, bgRect.top() + (bgRect.height() - cmdIconSize) / 2, cmdIconSize, cmdIconSize);
        painter->drawPixmap(cmdIconRect, QIcon::fromTheme("dt_command").pixmap(QSize(cmdIconSize, cmdIconSize)));

        if (option.state & QStyle::State_MouseOver) {
            QRect editIconRect = QRect(bgRect.right() - editIconSize - 6, bgRect.top() + (bgRect.height() - editIconSize) / 2,
                                       editIconSize, editIconSize);
            painter->drawPixmap(editIconRect, QIcon::fromTheme("dt_edit").pixmap(QSize(editIconSize, editIconSize)));
        }

        QString strCmdName = itemData.m_cmdName;
        QString strCmdShortcut = itemData.m_cmdShortcut;

        QFont textFont = painter->font();
        int cmdNameFontSize = DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T7);
        textFont.setPixelSize(cmdNameFontSize);
        strCmdName = Utils::getElidedText(textFont, strCmdName, ITEMMAXWIDTH);
        painter->setFont(textFont);

        int lineSpace = 8;
        int offsetY = 22 - cmdNameFontSize;
        int leftOffset = cmdIconRect.left() + cmdIconSize + 6;

        DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
        DPalette pa = appHelper->standardPalette(appHelper->themeType());
        painter->setPen(pa.color(DPalette::Text));

        QRect cmdNameRect = QRect(leftOffset, bgRect.top() + offsetY, bgRect.width() - cmdIconSize - editIconSize, 35);
        painter->drawText(cmdNameRect, Qt::AlignLeft | Qt::AlignTop, strCmdName);

        textFont.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T8));
        painter->setFont(textFont);

        DStyleHelper styleHelper;
        DPalette palette = DApplicationHelper::instance()->palette(m_parentView);
        painter->setPen(QPen(styleHelper.getColor(static_cast<const QStyleOption *>(&option), palette, DPalette::TextTips)));

        QRect cmdShortcutRect = QRect(leftOffset, cmdNameRect.top() + cmdNameFontSize + lineSpace,
                                      bgRect.width() - cmdIconSize - editIconSize, 35);
        painter->drawText(cmdShortcutRect, Qt::AlignLeft | Qt::AlignTop, strCmdShortcut);

        //tab键选择，绘制列表项外框
        if ((!m_bMouseOpt) && (option.state & QStyle::State_Selected)) {
            if (m_bModifyCheck) {
                //绘制编辑笔的选中框
                QRect rect = modifyRect;
                QPen framePen;
                DPalette pax = DApplicationHelper::instance()->palette(m_parentView);
                painter->setOpacity(1);
                framePen = QPen(pax.color(DPalette::Highlight), 2);
                painter->setPen(framePen);
                painter->drawEllipse(rect.left() + 1, rect.bottom() - rect.height() + 2, 18, 18);

            } else {
                //绘制列表项外框
                QPen framePen;

                DPalette pax = DApplicationHelper::instance()->palette(m_parentView);
                painter->setOpacity(1);
                framePen = QPen(pax.color(DPalette::Highlight), 2);

                painter->setPen(framePen);
                painter->drawPath(path);
            }

            //绘制编辑笔
            QRect editIconRect = QRect(bgRect.right() - editIconSize - 6, bgRect.top() + (bgRect.height() - editIconSize) / 2,
                                       editIconSize, editIconSize);
            painter->drawPixmap(editIconRect, QIcon::fromTheme("dt_edit").pixmap(QSize(editIconSize, editIconSize)));
        }

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
