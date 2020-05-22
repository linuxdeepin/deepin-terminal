#include "termbasedialog.h"
#include "utils.h"

#include <DFontSizeManager>
#include <DVerticalLine>
#include <DApplicationHelper>
#include <DGuiApplicationHelper>
#include <DLog>

DWIDGET_USE_NAMESPACE

TermBaseDialog::TermBaseDialog(QWidget *parent)
    : DAbstractDialog(parent)
{
    initUI();
    initConnections();
}

void TermBaseDialog::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 10);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    m_titleBar = new QWidget(this);
    m_titleBar->setFixedHeight(50);
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleBar->setLayout(titleLayout);

    m_logoIcon = new DLabel(this);
    m_logoIcon->setFixedSize(QSize(32, 32));
    m_logoIcon->setFocusPolicy(Qt::NoFocus);
    m_logoIcon->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_closeButton = new DWindowCloseButton(this);
    m_closeButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->setIconSize(QSize(50, 50));

    m_titleText = new DLabel(this);
    m_titleText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_titleText->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_titleText, DFontSizeManager::T5, QFont::DemiBold);
    // 字色
    DPalette palette = m_titleText->palette();
    palette.setColor(QPalette::WindowText, palette.color(DPalette::TextTitle));
    m_titleText->setPalette(palette);

    titleLayout->addWidget(m_logoIcon, 0, Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout->addWidget(m_titleText, 0, Qt::AlignHCenter);
    titleLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignTop);

    //Dialog content
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_content = new QWidget(this);
    m_content->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_content->setLayout(m_contentLayout);

    mainLayout->addWidget(m_titleBar, 0, Qt::AlignTop);
    mainLayout->addWidget(m_content);
    setLayout(mainLayout);

    m_mainLayout = mainLayout;
}

void TermBaseDialog::addCancelConfirmButtons()
{
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(9);
    buttonsLayout->setContentsMargins(10, 0, 10, 0);

    QFont btnFont;
    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setFixedWidth(209);
    m_cancelBtn->setFixedHeight(36);
    m_cancelBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_cancelBtn->setFont(btnFont);

    m_confirmBtn = new DSuggestButton(this);
    m_confirmBtn->setFixedWidth(209);
    m_confirmBtn->setFixedHeight(36);
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

    buttonsLayout->addWidget(m_cancelBtn);
    buttonsLayout->addWidget(verticalLine);
    buttonsLayout->addWidget(m_confirmBtn);
    /************************ Add by m000743 sunchengxi 2020-04-15:默认enter回车按下，走确认校验流程 Begin************************/
    m_confirmBtn->setDefault(true);
    /************************ Add by m000743 sunchengxi 2020-04-15:默认enter回车按下，走确认校验流程 End ************************/

    connect(m_cancelBtn, &DPushButton::clicked, this, [ = ]() {
        qDebug() << "cancelBtnClicked";
        m_confirmResultCode = QDialog::Rejected;
        reject();
        close();
    });

    connect(m_confirmBtn, &DPushButton::clicked, this, [ = ]() {
        qDebug() << "confirmBtnClicked";
        m_confirmResultCode = QDialog::Accepted;
        emit confirmBtnClicked();
    });

    m_mainLayout->addLayout(buttonsLayout);
}

QVBoxLayout *TermBaseDialog::getMainLayout()
{
    return m_mainLayout;
}

void TermBaseDialog::initConnections()
{
    connect(m_closeButton, &DWindowCloseButton::clicked, this, [this]() {
        this->close();
    });
}

QDialog::DialogCode TermBaseDialog::getConfirmResult()
{
    return m_confirmResultCode;
}

void TermBaseDialog::setLogoVisable(bool visible)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setVisible(visible);
    }
}

void TermBaseDialog::setTitle(const QString &title)
{
    if (nullptr != m_titleText) {
        m_titleText->setText(title);
    }
}

QLayout *TermBaseDialog::getContentLayout()
{
    return m_contentLayout;
}

void TermBaseDialog::setCancelBtnText(const QString &strCancel)
{
    m_cancelBtn->setText(strCancel);
    Utils::setSpaceInWord(m_cancelBtn);
}

void TermBaseDialog::setConfirmBtnText(const QString &strConfirm)
{
    m_confirmBtn->setText(strConfirm);
    Utils::setSpaceInWord(m_confirmBtn);
}

void TermBaseDialog::addContent(QWidget *content)
{
    Q_ASSERT(nullptr != getContentLayout());

    getContentLayout()->addWidget(content);
}

void TermBaseDialog::setIconPixmap(const QPixmap &iconPixmap)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setVisible(true);
        m_logoIcon->setPixmap(iconPixmap);
    }
}

void TermBaseDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    done(-1);

    Q_EMIT closed();
}
