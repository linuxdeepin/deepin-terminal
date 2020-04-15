#include "operationconfirmdlg.h"
#include "utils.h"

#include <DApplicationHelper>
#include <DGuiApplicationHelper>
#include <DButtonBox>
#include <DIconButton>
#include <DFontSizeManager>
#include <DVerticalLine>
#include <DLog>


OperationConfirmDlg::OperationConfirmDlg(QWidget *parent)
    : DAbstractDialog(parent)
{
    initUI();
    initContentLayout();
    initConnections();
}

void OperationConfirmDlg::initUI()
{
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(422, 202);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(10, 0, 0, 0);

    m_titleBar = new QWidget(this);
    m_titleBar->setFixedHeight(50);
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleBar->setLayout(titleLayout);

    m_logoIcon = new DLabel(this);
    m_logoIcon->setFixedSize(QSize(32, 32));
    m_logoIcon->setFocusPolicy(Qt::NoFocus);
    m_logoIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_logoIcon->setPixmap(QIcon::fromTheme("deepin-terminal").pixmap(QSize(32, 32)));

    m_closeButton = new DWindowCloseButton(this);
    m_closeButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->setIconSize(QSize(50, 50));

    m_titleText = new DLabel(this);
    m_titleText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titleText->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_titleText, DFontSizeManager::T6);

    titleLayout->addWidget(m_logoIcon, 0, Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout->addWidget(m_titleText);
    titleLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    //Dialog content
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_content = new QWidget(this);
    m_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_content->setLayout(m_contentLayout);

    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_content);
    setLayout(mainLayout);
}

void OperationConfirmDlg::initContentLayout()
{
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(10, 0, 10, 10);

    QWidget *mainFrame = new QWidget(this);
    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_operateTypeName = new DLabel(this);
    //m_operateTypeName->setFixedHeight(20);
    m_operateTypeName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 字色
    DPalette titlepalette = m_operateTypeName->palette();
    titlepalette.setBrush(QPalette::WindowText, titlepalette.color(DPalette::ToolTipText));
    m_operateTypeName->setPalette(titlepalette);
    // 字号
    DFontSizeManager::instance()->bind(m_operateTypeName, DFontSizeManager::T6, QFont::Medium);

    m_tipInfo = new DLabel(this);
    //m_tipInfo->setFixedHeight(20);
    m_tipInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 字号
    DFontSizeManager::instance()->bind(m_tipInfo, DFontSizeManager::T6, QFont::Normal);
    // 字色
    DPalette palette = m_tipInfo->palette();
    QColor color = DGuiApplicationHelper::adjustColor(palette.color(QPalette::WindowText), 0, 0, 0, 0, 0, 0, -30);
    palette.setColor(QPalette::WindowText, color);
    m_tipInfo->setPalette(palette);

    m_actionLayout = new QHBoxLayout();
    m_actionLayout->setSpacing(0);
    m_actionLayout->setContentsMargins(0, 0, 0, 0);

    QFont btnFont;
    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setFixedHeight(38);
    m_cancelBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_cancelBtn->setFont(btnFont);

    m_confirmBtn = new DWarningButton(this);
    m_confirmBtn->setFixedHeight(38);
    m_confirmBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_confirmBtn->setFont(btnFont);

    //设置回车键默认响应的按钮
    m_confirmBtn->setDefault(true);

    DVerticalLine *verticalLine = new DVerticalLine(this);
    DPalette pa = DApplicationHelper::instance()->palette(verticalLine);
    QColor splitColor = pa.color(DPalette::ItemBackground);
    pa.setBrush(DPalette::Background, splitColor);
    verticalLine->setPalette(pa);
    verticalLine->setBackgroundRole(QPalette::Background);
    verticalLine->setAutoFillBackground(true);
    verticalLine->setFixedSize(3, 28);

    m_actionLayout->addWidget(m_cancelBtn);
    m_actionLayout->addSpacing(8);
    m_actionLayout->addWidget(verticalLine);
    m_actionLayout->addSpacing(8);
    m_actionLayout->addWidget(m_confirmBtn);

    mainFrame->setLayout(m_mainLayout);

    addContent(mainFrame);
}

void OperationConfirmDlg::setDialogFrameSize(int width, int height)
{
    setFixedSize(width, height);
}

void OperationConfirmDlg::initConnections()
{
    connect(m_cancelBtn, &DPushButton::clicked, this, [ = ]() {
        qDebug() << "cancelBtnClicked";
        m_confirmResultCode = QDialog::Rejected;
        reject();
        close();
    });
    connect(m_confirmBtn, &DPushButton::clicked, this, [ = ]() {
        qDebug() << "confirmBtnClicked";
        m_confirmResultCode = QDialog::Accepted;
        close();
    });

    connect(m_closeButton, &DIconButton::clicked, this, [ = ]() {
        qDebug() << "dialog close Btn Clicked";
        m_confirmResultCode = QDialog::Rejected;
        close();
    });
}

void OperationConfirmDlg::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    done(-1);
    Q_EMIT closed();
}

void OperationConfirmDlg::setTitle(const QString &title)
{
    if (nullptr != m_titleText) {
        m_titleText->setText(title);
    }
}

QLayout *OperationConfirmDlg::getContentLayout()
{
    return m_contentLayout;
}

void OperationConfirmDlg::addContent(QWidget *content)
{
    Q_ASSERT(nullptr != getContentLayout());

    getContentLayout()->addWidget(content);
}

void OperationConfirmDlg::setIconPixmap(const QPixmap &iconPixmap)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setPixmap(iconPixmap);
    }
}

void OperationConfirmDlg::setOperatTypeName(const QString &strName)
{
    m_mainLayout->addWidget(m_operateTypeName, 0,  Qt::AlignVCenter | Qt::AlignHCenter);
    m_mainLayout->addSpacing(8);
    m_operateTypeName->setText(strName);
}

void OperationConfirmDlg::setTipInfo(const QString &strInfo)
{

    m_mainLayout->addWidget(m_tipInfo, 0, Qt::AlignVCenter | Qt::AlignHCenter);
    m_mainLayout->addStretch();
    m_tipInfo->setText(strInfo);
}

void OperationConfirmDlg::setOKCancelBtnText(const QString &strConfirm, const QString &strCancel)
{
    m_mainLayout->addLayout(m_actionLayout);
    m_confirmBtn->setText(strConfirm);
    m_cancelBtn->setText(strCancel);
    Utils::setSpaceInWord(m_confirmBtn);
    Utils::setSpaceInWord(m_cancelBtn);
}


QDialog::DialogCode OperationConfirmDlg::getConfirmResult()
{
    return m_confirmResultCode;
}
