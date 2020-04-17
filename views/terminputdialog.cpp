#include "terminputdialog.h"
#include "utils.h"
#include "termwidget.h"

#include <DFontSizeManager>
#include <DVerticalLine>
#include <DApplicationHelper>
#include <DGuiApplicationHelper>
#include <DLineEdit>

#include <DLog>

DWIDGET_USE_NAMESPACE

TermInputDialog::TermInputDialog(QWidget *parent)
    : DAbstractDialog(parent)
{
    initUI();
    initConnections();
}

void TermInputDialog::initUI()
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

    m_logoIcon = new DIconButton(this);
    m_logoIcon->setIconSize(QSize(32, 32));
    m_logoIcon->setWindowFlags(Qt::WindowTransparentForInput);
    m_logoIcon->setFocusPolicy(Qt::NoFocus);
    m_logoIcon->setFlat(true);
    // 默认无图标，所以隐藏
    m_logoIcon->hide();

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

    titleLayout->addSpacing(10);
    titleLayout->addWidget(m_logoIcon, 0, Qt::AlignLeading | Qt::AlignVCenter);
    titleLayout->addSpacing(50 - 32);
    titleLayout->addWidget(m_titleText);
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

void TermInputDialog::addCancelConfirmButtons(int width, int height, int topOffset, int outerSpace, int innerSpace)
{
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(innerSpace);
    buttonsLayout->setContentsMargins(outerSpace, 0, outerSpace, 0);

    QFont btnFont;
    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setFixedWidth(width);
    m_cancelBtn->setFixedHeight(height);
    m_cancelBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_cancelBtn->setFont(btnFont);

    m_confirmBtn = new DSuggestButton(this);
    m_confirmBtn->setFixedWidth(width);
    m_confirmBtn->setFixedHeight(height);
    m_confirmBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_confirmBtn->setFont(btnFont);

    //设置回车键默认响应的按钮
    m_confirmBtn->setDefault(true);

    DVerticalLine *verticalLine = new DVerticalLine(this);
    verticalLine->setFixedSize(1, 28);

    buttonsLayout->addWidget(m_cancelBtn);
    buttonsLayout->addWidget(verticalLine);
    buttonsLayout->addWidget(m_confirmBtn);
    /************************ Add by m000743 sunchengxi 2020-04-15:默认enter回车按下，走确认校验流程 Begin************************/
    m_confirmBtn->setDefault(true);
    /************************ Add by m000743 sunchengxi 2020-04-15:默认enter回车按下，走确认校验流程 End ************************/

//    connect(m_cancelBtn, &DPushButton::clicked, this, [ = ]() {
//        qDebug() << "cancelBtnClicked";
//        m_confirmResultCode = QDialog::Rejected;
//        reject();
//        close();
//    });

//    connect(m_confirmBtn, &DPushButton::clicked, this, [ = ]() {
//        qDebug() << "confirmBtnClicked";
//        m_confirmResultCode = QDialog::Accepted;
//        emit confirmBtnClicked();
//    });

    if (topOffset > 0) {
        m_mainLayout->addSpacing(topOffset);
    }
    m_mainLayout->addLayout(buttonsLayout);
}

void TermInputDialog::showDialog(QString oldTitle, QWidget *parentWidget)
{
    DLineEdit *lineEdit = new DLineEdit();
    lineEdit->setFixedSize(360, 36);
    lineEdit->setText(oldTitle);
    lineEdit->setClearButtonEnabled(false);
    lineEdit->setFocusPolicy(Qt::ClickFocus);
    this->setFocusProxy(lineEdit->lineEdit());

    // 初始化重命名窗口全选
    lineEdit->lineEdit()->selectAll();

    DLabel *label = new DLabel(tr("Tab name"));
    label->setFixedSize(360, 20);
    label->setAlignment(Qt::AlignHCenter);

    DPalette palette = label->palette();
    palette.setBrush(DPalette::WindowText, palette.color(DPalette::BrightText));
    label->setPalette(palette);

    // 字号
    DFontSizeManager::instance()->bind(label, DFontSizeManager::T6, QFont::Medium);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(10, 0, 10, 0);
    contentLayout->addWidget(label, Qt::AlignHCenter);
    contentLayout->addSpacing(8);
    contentLayout->addWidget(lineEdit, Qt::AlignHCenter);
    QWidget *contentWidget = new QWidget;
    contentWidget->setLayout(contentLayout);
    this->addContent(contentWidget);

    this->addCancelConfirmButtons(170, 36, 15, 10, 9);
    this->setCancelBtnText(tr("Cancel"));
    this->setConfirmBtnText(tr("OK"));

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

        TermWidget *termWidget = qobject_cast<TermWidget *>(parentWidget);
        emit termWidget->termRequestRenameTab(lineEdit->text());

        close();
    });

    this->exec();
}

QVBoxLayout *TermInputDialog::getMainLayout()
{
    return m_mainLayout;
}

void TermInputDialog::initConnections()
{
    connect(m_closeButton, &DWindowCloseButton::clicked, this, [this]() {
        this->close();
    });
}

QDialog::DialogCode TermInputDialog::getConfirmResult()
{
    return m_confirmResultCode;
}

void TermInputDialog::setLogoVisable(bool visible)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setVisible(visible);
    }
}

void TermInputDialog::setTitle(const QString &title)
{
    if (nullptr != m_titleText) {
        m_titleText->setText(title);
    }
}

QLayout *TermInputDialog::getContentLayout()
{
    return m_contentLayout;
}

void TermInputDialog::setCancelBtnText(const QString &strCancel)
{
    m_cancelBtn->setText(strCancel);
    Utils::setSpaceInWord(m_cancelBtn);
}

void TermInputDialog::setConfirmBtnText(const QString &strConfirm)
{
    m_confirmBtn->setText(strConfirm);
    Utils::setSpaceInWord(m_confirmBtn);
}

void TermInputDialog::addContent(QWidget *content)
{
    Q_ASSERT(nullptr != getContentLayout());

    getContentLayout()->addWidget(content);
}

void TermInputDialog::setIcon(const QIcon &icon)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setIcon(icon);
        m_logoIcon->setVisible(true);
    }
}

void TermInputDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    done(-1);

    Q_EMIT closed();
}
