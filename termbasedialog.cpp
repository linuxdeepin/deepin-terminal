#include "termbasedialog.h"

#include <DFontSizeManager>
#include <DVerticalLine>
#include <DApplicationHelper>
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
    QWidget *mainWidget = new QWidget(this);

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
    DFontSizeManager::instance()->bind(m_titleText, DFontSizeManager::T5);

    QFont titleFont = m_titleText->font();
    titleFont.setBold(true);
    m_titleText->setFont(titleFont);

    titleLayout->addWidget(m_logoIcon, 0, Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout->addWidget(m_titleText);
    titleLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    //Dialog content
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_content = new QWidget(this);
    m_content->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_content->setLayout(m_contentLayout);

    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_content);
    mainWidget->setLayout(mainLayout);

    m_mainLayout = mainLayout;
}

void TermBaseDialog::addCancelConfirmButtons()
{
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(0);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);

    QFont btnFont;
    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setFixedWidth(189);
    m_cancelBtn->setFixedHeight(38);
    m_cancelBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_cancelBtn->setFont(btnFont);

    m_confirmBtn = new DSuggestButton(this);
    m_confirmBtn->setFixedWidth(189);
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

    buttonsLayout->addSpacing(30);
    buttonsLayout->addWidget(m_cancelBtn);
    buttonsLayout->addSpacing(8);
    buttonsLayout->addWidget(verticalLine);
    buttonsLayout->addSpacing(8);
    buttonsLayout->addWidget(m_confirmBtn);
    buttonsLayout->addSpacing(30);

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
    connect(m_closeButton, &DWindowCloseButton::clicked, this,[this](){
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

void TermBaseDialog::setTitle(const QString& title)
{
    if(nullptr != m_titleText) {
        m_titleText->setText(title);
    }
}

QLayout* TermBaseDialog::getContentLayout()
{
    return m_contentLayout;
}

void TermBaseDialog::setCancelBtnText(const QString &strCancel)
{
    m_cancelBtn->setText(strCancel);
}

void TermBaseDialog::setConfirmBtnText(const QString &strConfirm)
{
    m_confirmBtn->setText(strConfirm);
}

void TermBaseDialog::addContent(QWidget* content)
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
