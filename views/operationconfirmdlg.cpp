#include "operationconfirmdlg.h"

#include <DApplicationHelper>
#include <DButtonBox>
#include <DIconButton>
#include <DFontSizeManager>
#include <DVerticalLine>
#include <DLog>

#include <QHBoxLayout>
#include <QVBoxLayout>

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
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 10, 10);

    QWidget *mainFrame = new QWidget(this);
    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_operateTypeName = new DLabel(this);
    m_operateTypeName->setFixedHeight(20);
    m_operateTypeName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QFont operateTypeNameFont;
    m_operateTypeName->setFont(operateTypeNameFont);
    DFontSizeManager::instance()->bind(m_operateTypeName, DFontSizeManager::T6);

    m_tipInfo = new DLabel(this);
    m_tipInfo->setFixedHeight(20);
    m_tipInfo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QFont tipInfoFont;
    m_tipInfo->setFont(tipInfoFont);
    DFontSizeManager::instance()->bind(m_tipInfo, DFontSizeManager::T6);
    DPalette paTipInfo = DApplicationHelper::instance()->palette(m_tipInfo);
    paTipInfo.setBrush(DPalette::WindowText, paTipInfo.color(DPalette::TextTips));
    m_tipInfo->setPalette(paTipInfo);

    QHBoxLayout *actionBarLayout = new QHBoxLayout();
    actionBarLayout->setSpacing(0);
    actionBarLayout->setContentsMargins(0, 0, 0, 0);

    QFont btnFont;
    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setFixedHeight(38);
    m_cancelBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_cancelBtn->setFont(btnFont);

    m_confirmBtn = new DWarningButton(this);
    m_confirmBtn->setFixedHeight(38);
    m_confirmBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_confirmBtn->setFont(btnFont);

    DVerticalLine *verticalLine = new DVerticalLine(this);
    DPalette pa = DApplicationHelper::instance()->palette(verticalLine);
    QColor splitColor = pa.color(DPalette::ItemBackground);
    pa.setBrush(DPalette::Background, splitColor);
    verticalLine->setPalette(pa);
    verticalLine->setBackgroundRole(QPalette::Background);
    verticalLine->setAutoFillBackground(true);
    verticalLine->setFixedSize(3, 28);

    actionBarLayout->addWidget(m_cancelBtn);
    actionBarLayout->addSpacing(8);
    actionBarLayout->addWidget(verticalLine);
    actionBarLayout->addSpacing(8);
    actionBarLayout->addWidget(m_confirmBtn);

    mainLayout->addWidget(m_operateTypeName, 0, Qt::AlignCenter);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(m_tipInfo, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    mainLayout->addLayout(actionBarLayout);
    mainFrame->setLayout(mainLayout);

    addContent(mainFrame);
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
    m_operateTypeName->setText(strName);
}

void OperationConfirmDlg::setTipInfo(const QString &strInfo)
{
    m_tipInfo->setText(strInfo);
}

void OperationConfirmDlg::setOKCancelBtnText(const QString &strConfirm, const QString &strCancel)
{
    m_confirmBtn->setText(strConfirm);
    m_cancelBtn->setText(strCancel);
}

QDialog::DialogCode OperationConfirmDlg::getConfirmResult()
{
    return m_confirmResultCode;
}
