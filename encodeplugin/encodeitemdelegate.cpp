#include "encodeitemdelegate.h"

#include <QPainter>
#include <DApplicationHelper>

EncodeItemDelegate::EncodeItemDelegate(QAbstractItemView *parent) : DStyledItemDelegate(parent) , m_parentView(parent)
{
}

void EncodeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QRect &rect = option.rect;
    const QString &encodeName = index.data(114514).toString();

    painter->setRenderHint(QPainter::Antialiasing, true);

    QFont font;
    font.setPointSize(13);
    painter->setFont(font);

    int paddingX = 10;
    int paddingY = 10;
    int m_frameRadius = 8;
    DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
    DPalette pa = DApplicationHelper::instance()->palette(m_parentView);

    // draw background.
    QPainterPath backgroundPath;
    backgroundPath.addRoundedRect(
    QRect(rect.x() + paddingX, rect.y() + paddingY, rect.width() - paddingX * 2, rect.height() - paddingY),
    m_frameRadius,
    m_frameRadius);

    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        painter->setOpacity(0.03);
    } else {
        painter->setOpacity(0.05);
    }
    painter->fillPath(backgroundPath, pa.color(DPalette::BrightText));

    // draw border frame.
    QPainterPath framePath;
    framePath.addRoundedRect(
    QRect(rect.x() + paddingX, rect.y() + paddingY, rect.width() - paddingX * 2-1, rect.height() - paddingY-1),
    m_frameRadius,
    m_frameRadius);
    QPen framePen;

    if (option.state & QStyle::State_Selected) {
        painter->setOpacity(1);
        framePen = QPen(pa.color(DPalette::Highlight), 2);
    }

    painter->setPen(framePen);
    painter->drawPath(framePath);

    // draw color theme prevew text.
    painter->setOpacity(1);
    QFontMetrics fm(font);
    int lineHeight = 50;

    int nameX = paddingX + 15;
    int nameY = paddingY + 10;
    painter->setPen(QPen(pa.color(DPalette::Foreground)));
    painter->drawText(
    QRect(rect.x() + nameX, rect.y() + nameY, rect.width(), lineHeight), Qt::AlignLeft | Qt::AlignTop, encodeName);
}

QSize EncodeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(-1, 60);
}
