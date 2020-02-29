#include "encodeitemdelegate.h"

#include <QPainter>
#include <DApplicationHelper>

EncodeItemDelegate::EncodeItemDelegate(QAbstractItemView *parent) : DStyledItemDelegate(parent)
{
}

void EncodeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //return;
    const QRect &rect = option.rect;
    const QString &encodeName = index.data(114514).toString();

    painter->setRenderHint(QPainter::Antialiasing, true);

    QFont font;
    font.setPointSize(10);
    painter->setFont(font);

    int paddingX = 15;
    int paddingY = 8;
    int m_frameRadius = 5;
    QString backgroundColor = "black";
    //QString frameSelectedColor = "green";
    //QString frameNormalColor = "blue";
    //QString otherColor = "white";
    DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
    DPalette pa = appHelper->standardPalette(appHelper->themeType());


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
        framePen = QPen(pa.color(DPalette::DarkLively), 2);
    } else {
        painter->setOpacity(0.3);
        framePen = QPen(pa.color(DPalette::FrameBorder), 1);
    }

    painter->setPen(framePen);
    painter->drawPath(framePath);

    // draw color theme prevew text.
    painter->setOpacity(1);
    QFontMetrics fm(font);
    int lineHeight = 20;

    int nameX = paddingX + 8;
    int nameY = paddingY + 8;
    painter->setPen(QPen(pa.color(DPalette::TextLively)));
    painter->drawText(
    QRect(rect.x() + nameX, rect.y() + nameY, rect.width(), lineHeight), Qt::AlignLeft | Qt::AlignTop, encodeName);
}

QSize EncodeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(-1, 61);
}
