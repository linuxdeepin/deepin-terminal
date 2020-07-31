#include "customcommandsearchrstpanel.h"
#include "customcommandoptdlg.h"
#include "shortcutmanager.h"
#include "iconbutton.h"

#include <DApplicationHelper>
#include <DGuiApplicationHelper>
#include <DMessageBox>

#include <QAction>

CustomCommandSearchRstPanel::CustomCommandSearchRstPanel(QWidget *parent)
    : CommonPanel(parent), m_cmdListWidget(new CustomCommandList())
{
    initUI();
}

/*******************************************************************************
 1. @函数:    initUI
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    初始化自定义搜索面板界面
*******************************************************************************/
void CustomCommandSearchRstPanel::initUI()
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    m_rebackButton = new IconButton(this);

    m_backButton = m_rebackButton;//m_backButton = new DIconButton(this);
    m_backButton->setIcon(DStyle::StandardPixmap::SP_ArrowLeave);
    m_backButton->setFixedSize(QSize(40, 40));
    m_backButton->setFocusPolicy(Qt::TabFocus);//m_backButton->setFocusPolicy(Qt::NoFocus);
    m_backButton->setFocus();
    //setTabOrder(m_backButton, m_cmdListWidget);

    m_label = new DLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    // 字体颜色随主题变化变化
    DPalette palette = m_label->palette();
    QColor color;
    if (DApplicationHelper::instance()->themeType() == DApplicationHelper::DarkType) {
        color = QColor::fromRgb(192, 198, 212, 102);
    } else {
        color = QColor::fromRgb(85, 85, 85, 102);
    }
    palette.setBrush(QPalette::Text, color);
    m_label->setPalette(palette);

    m_cmdListWidget->setSelectionMode(QAbstractItemView::SingleSelection);//m_cmdListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    m_cmdListWidget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
    m_cmdListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_cmdListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_cmdListWidget->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addSpacing(10);
    hlayout->addWidget(m_backButton);
    // 搜索框居中显示
    hlayout->addWidget(m_label, 0, Qt::AlignCenter);
    hlayout->setSpacing(0);
    hlayout->setMargin(0);

    QVBoxLayout *vlayout = new QVBoxLayout();
    vlayout->addSpacing(10);
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_cmdListWidget);
    vlayout->setMargin(0);
    vlayout->setSpacing(0);
    setLayout(vlayout);

    connect(m_cmdListWidget, &CustomCommandList::itemClicked, this, &CustomCommandSearchRstPanel::doCustomCommand);
    connect(m_backButton, &DIconButton::clicked, this, &CustomCommandSearchRstPanel::showCustomCommandPanel);
    connect(m_rebackButton, &IconButton::preFocus, this, [ = ]() {
        m_rebackButton->click();// 在派生类捕获方向键盘左键按下，转化为鼠标点击。
    });
    // 字体颜色随主题变化变化
    connect(DApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, m_label, [ = ](DGuiApplicationHelper::ColorType themeType) {
        DPalette palette = m_label->palette();
        QColor color;
        if (themeType == DApplicationHelper::DarkType) {
            color = QColor::fromRgb(192, 198, 212, 102);
        } else {
            color = QColor::fromRgb(85, 85, 85, 102);
        }
        palette.setBrush(QPalette::Text, color);
        m_label->setPalette(palette);
    });
}

/*******************************************************************************
 1. @函数:    setSearchFilter
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    设置搜索过滤条件及显示文本
*******************************************************************************/
void CustomCommandSearchRstPanel::setSearchFilter(const QString &filter)
{
    m_strFilter = filter;
    QString showText = Utils::getElidedText(m_label->font(), filter, ITEMMAXWIDTH, Qt::ElideMiddle);
    m_label->setText(QString("%1：%2").arg(tr("Search"), showText));
}

/*******************************************************************************
 1. @函数:    refreshData
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    根据 m_strFilter 刷新搜索面板自定义列表
*******************************************************************************/
void CustomCommandSearchRstPanel::refreshData()
{
    m_cmdListWidget->refreshCommandListData(m_strFilter);
}

/*******************************************************************************
 1. @函数:    refreshData
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    根据 strFilter 刷新搜索面板自定义列表
*******************************************************************************/
void CustomCommandSearchRstPanel::refreshData(const QString &strFilter)
{
    setSearchFilter(strFilter);
    m_cmdListWidget->refreshCommandListData(strFilter);
}

/*******************************************************************************
 1. @函数:    doCustomCommand
 2. @作者:    sunchengxi
 3. @日期:    2020-07-31
 4. @说明:    执行当前搜索面板自定义命令列表中itemData 中的自定义命令
*******************************************************************************/
void CustomCommandSearchRstPanel::doCustomCommand(CustomCommandItemData itemData, QModelIndex index)
{
    Q_UNUSED(index)

    QString strCommand = itemData.m_cmdText;
    if (!strCommand.endsWith('\n')) {
        strCommand.append('\n');
    }
    emit handleCustomCurCommand(strCommand);
    emit focusOut();
}

