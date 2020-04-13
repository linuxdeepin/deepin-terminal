#include "serverconfigdelegate.h"
#include "serverconfigitemmodel.h"
#include "utils.h"

#include <DApplicationHelper>
#include <DPalette>
#include <DStyleHelper>

#include <QPainter>
#include <QPixmap>
#include <DLog>

ServerConfigDelegate::ServerConfigDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_parentView(parent)
{
}

void ServerConfigDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QVariant varDisplay = index.data(Qt::DisplayRole);

        ServerConfigItemData itemData = varDisplay.value<ServerConfigItemData>();
        bool isgroup = isGroup(itemData);

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

        if (option.state & QStyle::State_MouseOver) {
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::ToolTipText);
            fillColor.setAlphaF(0.3);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, fillColor);
        } else {
            DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ItemBackground);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, fillColor);
        }

        int cmdIconSize = 44;
        int editIconSize = 20;

        QString themeType = "light";
        DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
        if (DGuiApplicationHelper::DarkType == appHelper->themeType()) {
            themeType = "dark";
        }
        QString strCmdIconSrc = QString(":/resources/images/buildin/%1/server.svg").arg(themeType);
        if (isgroup) {
            strCmdIconSrc = QString(":/resources/images/buildin/%1/server_group.svg").arg(themeType);
        }
        QPixmap cmdIconPixmap = Utils::renderSVG(strCmdIconSrc, QSize(cmdIconSize, cmdIconSize));

        QRect cmdIconRect = QRect(bgRect.left() + 8, bgRect.top() + (bgRect.height() - cmdIconSize) / 2,
                                  cmdIconSize, cmdIconSize);
        painter->drawPixmap(cmdIconRect, cmdIconPixmap);

        QString strServerName = itemData.m_serverName;
        QString strAddress = QString("%1@%2").arg(itemData.m_userName).arg(itemData.m_address);

        if (isgroup) {
            strServerName = itemData.m_group;
            strAddress = itemData.m_number;
            editIconSize = 12;
            QString strEditIconSrc = QString(":/resources/images/buildin/%1/arrow_right.svg").arg(themeType);
            //qDebug() << strEditIconSrc << endl;
            QPixmap editIconPixmap = Utils::renderSVG(strEditIconSrc, QSize(editIconSize, editIconSize));
            QRect editIconRect = QRect(bgRect.right() - editIconSize - 6, bgRect.top() + (bgRect.height() - editIconSize) / 2,
                                       editIconSize, editIconSize);
            painter->drawPixmap(editIconRect, editIconPixmap);
        } else {
            if (option.state & QStyle::State_MouseOver) {
                QString strEditIconSrc = QString(":/resources/images/buildin/%1/edit.svg").arg(themeType);
                QPixmap editIconPixmap = Utils::renderSVG(strEditIconSrc, QSize(editIconSize, editIconSize));
                QRect editIconRect = QRect(bgRect.right() - editIconSize - 6, bgRect.top() + (bgRect.height() - editIconSize) / 2,
                                           editIconSize, editIconSize);
                painter->drawPixmap(editIconRect, editIconPixmap);
            }
        }

        QFont textFont = painter->font();
        int textFontSize = DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T7);
        textFont.setPixelSize(textFontSize);
        strServerName = Utils::getElidedText(textFont, strServerName, ITEMMAXWIDTH);
        painter->setFont(textFont);

        int lineSpace = 8;
        int offsetY = 8;
        int leftOffset = cmdIconRect.left() + cmdIconSize + 6;

        DPalette pa = appHelper->standardPalette(appHelper->themeType());
        painter->setPen(pa.color(DPalette::Text));

        QRect serverNameRect = QRect(leftOffset, bgRect.top()+offsetY, bgRect.width() - cmdIconSize - editIconSize, 35);
        painter->drawText(serverNameRect, Qt::AlignLeft | Qt::AlignTop, strServerName);

        textFont.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T8));
        strAddress = Utils::getElidedText(textFont, strAddress, ITEMMAXWIDTH);
        painter->setFont(textFont);

        DStyleHelper styleHelper;
        DPalette palette = DApplicationHelper::instance()->palette(m_parentView);
        painter->setPen(QPen(styleHelper.getColor(static_cast<const QStyleOption *>(&option), palette, DPalette::TextTips)));

        QRect addressRect = QRect(leftOffset, serverNameRect.top() + textFontSize + lineSpace, bgRect.width() - cmdIconSize - editIconSize, 35);
        painter->drawText(addressRect, Qt::AlignLeft | Qt::AlignTop, strAddress);

        painter->restore();
    } else {
        DStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ServerConfigDelegate::sizeHint(const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    Q_UNUSED(index)

    return QSize(option.rect.width(), 70);
}

/**
 * @brief 判断是否分组方式显示(根据配置信息中分组字段是否为空),如果已经在分组界面，直接返回false
 * @param itemData
 * @return
 */
bool ServerConfigDelegate::isGroup(ServerConfigItemData &itemData) const
{
    if (itemData.m_IsInGrouppanel) {
        return false;
    }

    return itemData.m_IsGroupItem;
}
