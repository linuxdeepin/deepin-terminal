#include "themeitemdelegate.h"

#include <QPainter>

ThemeItemDelegate::ThemeItemDelegate(QAbstractItemView *parent) : DStyledItemDelegate(parent)
{
}

void ThemeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QRect &rect = option.rect;
    const QString &themeName = index.data(114514).toString();

    painter->setRenderHint(QPainter::Antialiasing, true);

    QFont font;
    font.setPointSize(10);
    painter->setFont(font);

    int paddingX = 15;
    int paddingY = 8;
    int m_frameRadius = 5;
    QString backgroundColor = "black";
    QString frameSelectedColor = "green";
    QString frameNormalColor = "blue";
    QString otherColor = "white";

    // draw background.
    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(
    QRect(rect.x() + paddingX, rect.y() + paddingY, rect.width() - paddingX * 2, rect.height() - paddingY * 2),
    m_frameRadius,
    m_frameRadius);

    painter->setOpacity(0.8);
    painter->fillPath(backgroundPath, QColor(backgroundColor));

    // draw border frame.
    QPainterPath framePath;
    framePath.addRoundedRect(
    QRect(rect.x() + paddingX, rect.y() + paddingY, rect.width() - paddingX * 2, rect.height() - paddingY * 2),
    m_frameRadius,
    m_frameRadius);
    QPen framePen;

    if (option.state & QStyle::State_Selected) {
        painter->setOpacity(1);
        framePen = QPen(QColor(frameSelectedColor), 2);
    } else {
        painter->setOpacity(0.3);
        framePen = QPen(QColor(frameNormalColor), 1);
    }

    painter->setPen(framePen);
    painter->drawPath(framePath);

    // draw color theme prevew text.
    painter->setOpacity(1);
    QFontMetrics fm(font);
    int lineHeight = 20;

    int nameX = paddingX + 8;
    int nameY = paddingY + 8;
    painter->setPen(QPen(QColor(otherColor)));
    painter->drawText(
    QRect(rect.x() + nameX, rect.y() + nameY, rect.width(), lineHeight), Qt::AlignLeft | Qt::AlignTop, themeName);
}

QSize ThemeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(-1, 61);
}
