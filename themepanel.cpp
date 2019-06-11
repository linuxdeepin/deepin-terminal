#include "themepanel.h"

#include "themelistview.h"
#include "themelistmodel.h"
#include "themeitemdelegate.h"

#include <QDebug>
#include <QScroller>
#include <QVBoxLayout>

ThemePanel::ThemePanel(QWidget *parent)
    : RightPanel(parent),
      m_themeView(new ThemeListView(this)),
      m_themeModel(new ThemeListModel(this))
{
    // init view.
    m_themeView->setModel(m_themeModel);
    m_themeView->setItemDelegate(new ThemeItemDelegate(this));

    setFocusProxy(m_themeView);
    QScroller::grabGesture(m_themeView, QScroller::TouchGesture);

    // init layout.
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_themeView);

    layout->setMargin(0);
    layout->setSpacing(0);

    connect(m_themeView, &ThemeListView::focusOut, this, &RightPanel::hide);
    connect(m_themeView, &ThemeListView::themeChanged, this, &ThemePanel::themeChanged);
}
