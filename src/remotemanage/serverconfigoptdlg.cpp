// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "serverconfigoptdlg.h"
#include "serverconfigmanager.h"
#include "termcommandlinkbutton.h"
#include "utils.h"

//dtk
#include <DFontSizeManager>
#include <DPushButton>
#include <DSuggestButton>
#include <DGuiApplicationHelper>
#include <DPaletteHelper>
#include <DFileDialog>
#include <DPalette>
#include <DDialog>

//qt
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextCodec>
#include <QSpacerItem>
#include <QTimer>
#include <QDebug>
#include <QLoggingCategory>
//c++
#include <iterator>//added byq qinyaning

DGUI_USE_NAMESPACE
Q_DECLARE_LOGGING_CATEGORY(remotemanage)

ServerConfigOptDlg::ServerConfigOptDlg(ServerConfigOptType type, ServerConfig *curServer, QWidget *parent)
    : DAbstractDialog(true, parent),
      m_type(type),
      m_curServer(curServer),
      m_titleLabel(new DLabel),
      m_iconLabel(new DLabel),
      m_serverName(new DLineEdit(this)),
      m_closeButton(new DWindowCloseButton(this)),
      m_address(new DLineEdit(this)),
      m_port(new DSpinBox(this)),
      m_port_tip(new DAlertControl(m_port, this)),
      m_userName(new DLineEdit(this)),
      m_password(new DPasswordEdit(this)),
      m_privateKey(new DFileChooserEdit(this)),
      m_group(new DComboBox(this)),
      m_path(new DLineEdit(this)),
      m_command(new DLineEdit(this)),
      m_coding(new DComboBox(this)),
      m_backSapceKey(new DComboBox(this)),
      m_deleteKey(new DComboBox(this)),
      m_pGridLayout(new QGridLayout),
      m_advancedOptions(new DCommandLinkButton(tr("Advanced options"), this)),
      m_delServer(new TermCommandLinkButton(this))
{
    // qDebug() << "Enter ServerConfigOptDlg::ServerConfigOptDlg";
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 Begin***************/
    Utils::set_Object_Name(this);
    m_titleLabel->setObjectName("RemoteTitleLabel");
    m_iconLabel->setObjectName("RemoteIconLabel");
    m_closeButton->setObjectName("RemoteCloseButton");
    m_serverName->setObjectName("RemoteServerNameLineEdit");
    m_address->setObjectName("RemoteAddressLineEdit");
    m_port->setObjectName("RemotePortDSpinBox");
    m_userName->setObjectName("RemoteUserNameLineEdit");
    m_password->setObjectName("RemoteDPasswordEdit");
    m_privateKey->setObjectName("RemotePrivateKeyLineEdit");
    m_group->setObjectName("RemoteGroupComboBox");
    m_path->setObjectName("RemotePathLineEdit");
    m_command->setObjectName("RemoteCommandLineEdit");
    m_coding->setObjectName("RemoteEncodeComboBox");
    m_backSapceKey->setObjectName("RemoteBackSapceKeyComboBox");
    m_deleteKey->setObjectName("RemoteDeleteKeyComboBox");
    m_advancedOptions->setObjectName("RemoteAdvancedOptionsButton");
    m_delServer->setObjectName("RemoteDelServerButton");
    /******** Add by ut001000 renfeixiang 2020-08-13:增加 End***************/
    setWindowModality(Qt::WindowModal);
    setAutoFillBackground(true);
    initUI();
    initData();
}

void ServerConfigOptDlg::initUI()
{
    qCDebug(remotemanage) << "Enter initUI";
    //all layout
    QVBoxLayout *m_VBoxLayout = new QVBoxLayout();
    m_VBoxLayout->setSpacing(SPACEHEIGHT);
    m_VBoxLayout->setContentsMargins(0, 0, 0, SPACEHEIGHT);
    //head layout
    m_iconLabel->setFixedSize(ICONSIZE_50, ICONSIZE_50);
    m_iconLabel->setPixmap(QIcon(QStringLiteral(":/logo/deepin-terminal.svg")).pixmap(ICONSIZE_36, ICONSIZE_36));
    m_titleLabel->setText(tr("Add Server"));
    m_titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    m_closeButton->setFocusPolicy(Qt::TabFocus);
    m_closeButton->setFixedWidth(ICONSIZE_50);
    m_closeButton->setIconSize(QSize(ICONSIZE_50, ICONSIZE_50));

    // 字体
    DFontSizeManager::instance()->bind(m_titleLabel, DFontSizeManager::T5, QFont::DemiBold);
    // 字色
    DPalette palette = m_titleLabel->palette();
    QColor color;
    if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
        qCDebug(remotemanage) << "dark theme type";
        color = QColor::fromRgb(192, 198, 212, 255);
    } else {
        qCDebug(remotemanage) << "light theme type";
        color = QColor::fromRgb(0, 26, 46, 255);
    }

    palette.setBrush(QPalette::WindowText, color);
    m_titleLabel->setPalette(palette);

    QHBoxLayout *headLayout = new QHBoxLayout();
    headLayout->setSpacing(0);
    headLayout->setContentsMargins(12, 0, 0, 0);
    // 为了和closebutton对应是标题居中
    headLayout->addWidget(m_iconLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    headLayout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    headLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignTop);
    connect(m_closeButton, &DDialogCloseButton::clicked, this, [ = ]() {
        qDebug() << "Lambda: close button clicked";
        reject();
    });
    m_VBoxLayout->addLayout(headLayout);
    //main layout
    m_pGridLayout->setContentsMargins(28, 0, 30, 0);
    m_pGridLayout->setSpacing(SPACEHEIGHT);

    //m_serverName
    DLabel *pServerNameLabel = new DLabel(tr("Server name:"));
    setLabelStyle(pServerNameLabel);
    m_serverName->lineEdit()->setPlaceholderText(tr("Required"));
    QTimer::singleShot(30, this, [&]() {
        qDebug() << "Lambda: select all server name text";
        m_serverName->lineEdit()->selectAll();
    });
    m_pGridLayout->addWidget(pServerNameLabel, 0, 0);
    m_pGridLayout->addWidget(m_serverName, 0, 1, 1, 3);

    //pAddressLabel
    DLabel *pAddressLabel = new DLabel(tr("Address:"));
    setLabelStyle(pAddressLabel);
    m_address->lineEdit()->setPlaceholderText(tr("Required"));
    m_pGridLayout->addWidget(pAddressLabel, 1, 0);
    m_pGridLayout->addWidget(m_address, 1, 1);

    //pPortLabel
    DLabel *pPortLabel = new DLabel(tr("Port:"));
    pPortLabel->setAlignment(Qt::AlignLeft);
    pPortLabel->setAlignment(Qt::AlignVCenter);
    pPortLabel->setMinimumWidth(40);
    DFontSizeManager::instance()->bind(pPortLabel, DFontSizeManager::T6);
    m_port->setRange(0, 65535);
    m_port->setValue(DEFAULTPORT);
    m_port->setSingleStep(1);
    m_port->setFixedWidth(70);
    //comment code for old version dtk build
#if (DTK_VERSION_MAJOR > 5 || (DTK_VERSION_MAJOR >= 5 && (DTK_VERSION_MINOR > 1 || (DTK_VERSION_MINOR >= 1 && DTK_VERSION_BUILD >= 2))))
    m_port->lineEdit()->setClearButtonEnabled(false);
#endif
    // 去除上下按钮
    m_port->setButtonSymbols(DSpinBox::NoButtons);
    // 禁用输入法
    m_port->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_pGridLayout->addWidget(pPortLabel, 1, 2);
    m_pGridLayout->addWidget(m_port, 1, 3);

    //pUsernameLabel
    DLabel *pUsernameLabel = new DLabel(tr("Username:"));
    setLabelStyle(pUsernameLabel);
    m_userName->lineEdit()->setPlaceholderText(tr("Required"));
    m_pGridLayout->addWidget(pUsernameLabel, 2, 0);
    m_pGridLayout->addWidget(m_userName, 2, 1, 1, 3);

    //pPasswordLabel
    DLabel *pPasswordLabel = new DLabel(tr("Password:"));
    setLabelStyle(pPasswordLabel);
    m_password->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);
    m_pGridLayout->addWidget(pPasswordLabel, 3, 0);
    m_pGridLayout->addWidget(m_password, 3, 1, 1, 3);

    DLabel *pPrivateKeyLabel = new DLabel(tr("Certificate:"));
    setLabelStyle(pPrivateKeyLabel);

    //pPrivateKeyLabel
    m_pGridLayout->addWidget(pPrivateKeyLabel, 4, 0);
    m_pGridLayout->addWidget(m_privateKey, 4, 1, 1, 3);

    //m_advancedOptions
    palette = DPaletteHelper::instance()->palette(m_advancedOptions);
    palette.setColor(DPalette::ButtonText, palette.color(DPalette::Highlight));
    m_advancedOptions->setPalette(palette);
    m_advancedOptions->setFocusPolicy(Qt::TabFocus);
    m_pGridLayout->addWidget(m_advancedOptions, 5, 0, 1, 4, Qt::AlignCenter);

    //senior layout
    DLabel *pGroupLabel = new DLabel(tr("Group:"));
    setLabelStyle(pGroupLabel);
    m_pGridLayout->addWidget(pGroupLabel, 6, 0);
    m_group->setEditable(true);
    m_group->lineEdit()->setPlaceholderText(tr("No Group"));
    m_pGridLayout->addWidget(m_group, 6, 1, 1, 3);

    //pPathLabel
    DLabel *pPathLabel = new DLabel(tr("Path:"));
    setLabelStyle(pPathLabel);
    m_pGridLayout->addWidget(pPathLabel, 7, 0);
    m_pGridLayout->addWidget(m_path, 7, 1, 1, 3);

    //pCommandLabel
    DLabel *pCommandLabel = new DLabel(tr("Command:"));
    setLabelStyle(pCommandLabel);
    m_pGridLayout->addWidget(pCommandLabel, 8, 0);
    m_pGridLayout->addWidget(m_command, 8, 1, 1, 3);

    //pCodingLabel
    DLabel *pCodingLabel = new DLabel(tr("Encoding:"));
    setLabelStyle(pCodingLabel);
    m_pGridLayout->addWidget(pCodingLabel, 9, 0);
    m_pGridLayout->addWidget(m_coding, 9, 1, 1, 3);

    //pBackspaceKeyLabel
    DLabel *pBackspaceKeyLabel = new DLabel(tr("Backspace key:"));
    setLabelStyle(pBackspaceKeyLabel);
    m_pGridLayout->addWidget(pBackspaceKeyLabel, 10, 0);
    m_pGridLayout->addWidget(m_backSapceKey, 10, 1, 1, 3);

    //pDeleteKeyLabel
    DLabel *pDeleteKeyLabel = new DLabel(tr("Delete key:"));
    setLabelStyle(pDeleteKeyLabel);
    m_pGridLayout->addWidget(pDeleteKeyLabel, 11, 0);
    m_pGridLayout->addWidget(m_deleteKey, 11, 1, 1, 3);

    //m_delServer
    m_delServer->setText(tr("Delete server"));
    m_pGridLayout->addWidget(m_delServer, 12, 0, 1, 4, Qt::AlignCenter);

    m_VBoxLayout->addLayout(m_pGridLayout, 1);

    //hide after m_advancedOptions
    setAdvanceRegionVisible(false);

    DFontSizeManager::instance()->bind(m_advancedOptions, DFontSizeManager::T8, QFont::Normal);
    connect(m_advancedOptions, &DCommandLinkButton::clicked, this, [ = ]() {
        qDebug() << "Lambda: advanced options clicked";
        setAdvanceRegionVisible(true);

    });
    m_VBoxLayout->addLayout(m_pGridLayout);

    DPushButton *pCancelButton = new DPushButton(tr("Cancel", "button"));
    DSuggestButton *pAddSaveButton = new DSuggestButton(tr("Add", "button"));
    //Add a line by m000750 zhangmeng 2020-04-22设置回车触发默认按钮
    pAddSaveButton->setDefault(true);
    if (SCT_MODIFY == m_type) {
        qCDebug(remotemanage) << "modify server type";
        m_titleLabel->setText(tr("Edit Server"));
        pAddSaveButton->setText(tr("Save", "button"));
    }
    Utils::setSpaceInWord(pCancelButton);
    Utils::setSpaceInWord(pAddSaveButton);
    DPalette pa = DPaletteHelper::instance()->palette(pAddSaveButton);
    QBrush brush = pa.textLively().color();
    pa.setBrush(DPalette::ButtonText, brush);
    pAddSaveButton->setPalette(pa);
    QHBoxLayout *pBtHbLayout = new QHBoxLayout();
    m_bottomVLine = new DVerticalLine();
    m_bottomVLine->setFixedSize(3, 28);
    pBtHbLayout->setContentsMargins(10, 0, 10, 0);
    pBtHbLayout->setSpacing(9);
    pBtHbLayout->addWidget(pCancelButton);
    pBtHbLayout->addWidget(m_bottomVLine);
    pBtHbLayout->addWidget(pAddSaveButton);
    m_VBoxLayout->addLayout(pBtHbLayout);
    setLayout(m_VBoxLayout);

    connect(pCancelButton, &DPushButton::clicked, this, [ = ]() {
        qDebug() << "Lambda: cancel button clicked";
        reject();
    });
    connect(pAddSaveButton, &DPushButton::clicked, this, &ServerConfigOptDlg::slotAddSaveButtonClicked);
    connect(m_delServer, &TermCommandLinkButton::clicked, this, [ = ]() {
        qDebug() << "Lambda: delete server clicked";
        setDelServer(true);
        accept();
    });
    // 字体颜色随主题变化变化
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ServerConfigOptDlg::handleThemeTypeChanged);

    // 设置焦点顺序
    setTabOrder(pAddSaveButton, m_closeButton);

#ifdef DTKWIDGET_CLASS_DSizeMode
    updateSizeMode();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &ServerConfigOptDlg::updateSizeMode);
    // 仅在紧凑模式下处理，此模式下调整字体大小可能导致布局间距存在差异。
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::fontChanged, this, [this](){
        qDebug() << "Lambda: font changed";
        if (DGuiApplicationHelper::isCompactMode()) {
            qCDebug(remotemanage) << "compact mode font change";
            QTimer::singleShot(0, this, [=](){ resize(SETTING_DIALOG_WIDTH, sizeHint().height()); });
        }
    });
#endif
    qCDebug(remotemanage) << "Exit initUI";
}

inline void ServerConfigOptDlg::handleThemeTypeChanged(DGuiApplicationHelper::ColorType themeType)
{
    // qDebug() << "Enter ServerConfigOptDlg::handleThemeTypeChanged";
    DPalette palette = m_titleLabel->palette();
    //palette.setBrush(QPalette::WindowText, palette.color(DPalette::TextTitle));
    QColor color;
    if (DGuiApplicationHelper::DarkType == themeType) {
        qCDebug(remotemanage) << "dark theme type";
        color = QColor::fromRgb(192, 198, 212, 255);
    } else {
        qCDebug(remotemanage) << "light theme type";
        color = QColor::fromRgb(0, 26, 46, 255);
    }

    palette.setBrush(QPalette::WindowText, color);
    m_titleLabel->setPalette(palette);

    palette = DPaletteHelper::instance()->palette(m_delServer);
    palette.setColor(DPalette::ButtonText, palette.color(DPalette::TextWarning));
    m_delServer->setPalette(palette);
}

/**
 * @brief 接收 DGuiApplicationHelper::sizeModeChanged() 信号, 根据不同的布局模式调整
 *      当前界面的布局. 只能在界面创建完成后调用.
 *      在此界面中调整标题栏组件、文本Label的属性，调整后修改设置框界面大小。
 */
void ServerConfigOptDlg::updateSizeMode()
{
    // qDebug() << "Enter ServerConfigOptDlg::updateSizeMode";
#ifdef DTKWIDGET_CLASS_DSizeMode
    QList<DLabel *> labelList = findChildren<DLabel *>();

    if (DGuiApplicationHelper::isCompactMode()) {
        qCDebug(remotemanage) << "compact mode";
        for (DLabel *label : labelList) {
            label->setFixedHeight(COMMONHEIGHT_COMPACT);
        }

        m_iconLabel->setFixedSize(ICONSIZE_40_COMPACT, ICONSIZE_40_COMPACT);
        m_closeButton->setFixedWidth(ICONSIZE_40_COMPACT);
        m_closeButton->setIconSize(QSize(ICONSIZE_40_COMPACT, ICONSIZE_40_COMPACT));
        DFontSizeManager::instance()->bind(m_advancedOptions, DFontSizeManager::T6, QFont::Normal);
        m_bottomVLine->setFixedSize(VERTICAL_WIDTH_COMPACT, VERTICAL_HEIGHT_COMPACT);
    } else {
        qCDebug(remotemanage) << "normal mode";
        for (DLabel *label : labelList) {
            label->setMinimumHeight(COMMONHEIGHT);
        }

        m_iconLabel->setFixedSize(ICONSIZE_50, ICONSIZE_50);
        m_closeButton->setFixedWidth(ICONSIZE_50);
        m_closeButton->setIconSize(QSize(ICONSIZE_50, ICONSIZE_50));
        DFontSizeManager::instance()->bind(m_advancedOptions, DFontSizeManager::T8, QFont::Normal);
        m_bottomVLine->setFixedSize(VERTICAL_WIDTH, VERTICAL_HEIGHT);
    }

    // 根据新界面布局，刷新界面大小
    QTimer::singleShot(0, this, [=](){ resize(SETTING_DIALOG_WIDTH, sizeHint().height()); });
#endif
}

void ServerConfigOptDlg::initData()
{
    // qDebug() << "Enter ServerConfigOptDlg::initData";
    qCDebug(remotemanage) << "Enter initData";
    QList<QString> groupList = ServerConfigManager::instance()->getServerConfigs().keys();
    m_group->addItems(groupList);
    m_group->setCurrentIndex(-1);
    QList<QString> textCodeList = getTextCodec();
    m_coding->addItems(textCodeList);
    QList<QString> backSpaceKeyList = getBackSpaceKey();
    m_backSapceKey->addItems(backSpaceKeyList);
    QList<QString> deleteKeyList = getDeleteKey();
    m_deleteKey->addItems(deleteKeyList);
    if (SCT_MODIFY == m_type && m_curServer != nullptr) {
        qCDebug(remotemanage) << "modify server with valid server config";
        m_serverName->setText(m_curServer->m_serverName);
        m_currentServerName = m_serverName->text();
        m_address->setText(m_curServer->m_address);
        m_port->setValue(m_curServer->m_port.toInt());
        m_userName->setText(m_curServer->m_userName);
        m_password->setText(m_curServer->m_password);
        m_privateKey->setText(m_curServer->m_privateKey);
        m_path->setText(m_curServer->m_path);
        m_command->setText(m_curServer->m_command);
        m_group->setCurrentText(m_curServer->m_group);
        if (!m_curServer->m_encoding.isEmpty()) {
            qCDebug(remotemanage) << "setting encoding";
            int textCodeIndex = textCodeList.indexOf(m_curServer->m_encoding);
            m_coding->setCurrentIndex(textCodeIndex);
        }
        if (!m_curServer->m_backspaceKey.isEmpty()) {
            qCDebug(remotemanage) << "setting backspace key";
            int backSpaceKeyIndex = backSpaceKeyList.indexOf(m_curServer->m_backspaceKey);
            m_backSapceKey->setCurrentIndex(backSpaceKeyIndex);
        }
        if (!m_curServer->m_deleteKey.isEmpty()) {
            qCDebug(remotemanage) << "setting delete key";
            int deleteKeyIndex = deleteKeyList.indexOf(m_curServer->m_deleteKey);
            m_deleteKey->setCurrentIndex(deleteKeyIndex);
        }
    }
    qCDebug(remotemanage) << "Exit initData";
}

QList<QString> ServerConfigOptDlg::getTextCodec()
{
    // qDebug() << "Enter ServerConfigOptDlg::getTextCodec";
    /******** Modify by ut000610 daizhengwen 2020-05-28: 编码列表和编码插件一致****************/
    //    QList<QByteArray> list = QTextCodec::availableCodecs();
    QList<QByteArray> list = Utils::encodeList();
    /********************* Modify by ut000610 daizhengwen End ************************/
    QList<QString> textCodecList;
    for (QByteArray &byteArr : list) {
        QString str = QString(byteArr);
        if (!textCodecList.contains(str)) {
            qCDebug(remotemanage) << "adding new codec";
            textCodecList.append(str);
        }
    }
    return textCodecList;
}

QList<QString> ServerConfigOptDlg::getBackSpaceKey()
{
    // qDebug() << "Enter ServerConfigOptDlg::getBackSpaceKey";
    QList<QString> eraseKeyList;
    eraseKeyList.append(("ascii-del"));
    eraseKeyList.append(("auto"));
    eraseKeyList.append(("control-h"));
    eraseKeyList.append(("escape-sequence"));
    eraseKeyList.append(("tty"));
    return eraseKeyList;
}

QList<QString> ServerConfigOptDlg::getDeleteKey()
{
    // qDebug() << "Enter ServerConfigOptDlg::getDeleteKey";
    QList<QString> eraseKeyList;
    eraseKeyList.append(("escape-sequence"));
    eraseKeyList.append(("ascii-del"));
    eraseKeyList.append(("auto"));
    eraseKeyList.append(("control-h"));
    eraseKeyList.append(("tty"));
    return eraseKeyList;
}

void ServerConfigOptDlg::setLabelStyle(DLabel *label)
{
    // qDebug() << "Enter ServerConfigOptDlg::setLabelStyle";
    label->setAlignment(Qt::AlignLeft);
    label->setAlignment(Qt::AlignVCenter);
    label->setMinimumHeight(36);

    DFontSizeManager::instance()->bind(label, DFontSizeManager::T6);
}

ServerConfigOptDlg::~ServerConfigOptDlg()
{
    // qDebug() << "Enter ServerConfigOptDlg::~ServerConfigOptDlg";
}

void ServerConfigOptDlg::updataData(ServerConfig *curServer)
{
    // qDebug() << "Enter ServerConfigOptDlg::updataData";
    qCDebug(remotemanage) << "ServerConfigOptDlg server configuration options updata data.";
    // 读取配置
    QList<QString> textCodeList = getTextCodec();
    QList<QString> backSpaceKeyList = getBackSpaceKey();
    QList<QString> deleteKeyList = getDeleteKey();
    // 重置m_curServer
//    resetCurServer(curServer);
    m_curServer = curServer;
    // 设置数据
    m_serverName->setText(curServer->m_serverName);
    m_address->setText(curServer->m_address);
    m_port->setValue(curServer->m_port.toInt());
    m_userName->setText(curServer->m_userName);
    m_password->setText(curServer->m_password);
    m_privateKey->setText(curServer->m_privateKey);
    m_path->setText(curServer->m_path);
    m_command->setText(curServer->m_command);
    m_group->setCurrentText(m_curServer->m_group);
    if (!curServer->m_encoding.isEmpty()) {
        qCDebug(remotemanage) << "setting encoding";
        int textCodeIndex = textCodeList.indexOf(curServer->m_encoding);
        m_coding->setCurrentIndex(textCodeIndex);
    }
    if (!curServer->m_backspaceKey.isEmpty()) {
        qCDebug(remotemanage) << "setting backspace key";
        int backSpaceKeyIndex = backSpaceKeyList.indexOf(curServer->m_backspaceKey);
        m_backSapceKey->setCurrentIndex(backSpaceKeyIndex);
    }
    if (!curServer->m_deleteKey.isEmpty()) {
        qCDebug(remotemanage) << "setting delete key";
        int deleteKeyIndex = deleteKeyList.indexOf(curServer->m_deleteKey);
        m_deleteKey->setCurrentIndex(deleteKeyIndex);
    }

    m_currentServerName = m_serverName->text();
    qCDebug(remotemanage) << "Exit updataData";
}

ServerConfig ServerConfigOptDlg::getData()
{
    // qDebug() << "Enter ServerConfigOptDlg::getData";
    ServerConfig config;
    config.m_serverName = m_serverName->text();
    config.m_address = m_address->text();
    config.m_userName = m_userName->text();
    config.m_password = m_password->text();
    config.m_privateKey = m_privateKey->text();
    config.m_port = m_port->text();
    config.m_group = m_group->currentText();
    config.m_path = m_path->text();
    config.m_command = m_command->text();
    config.m_encoding = m_coding->currentText();
    config.m_backspaceKey = m_backSapceKey->currentText();
    config.m_deleteKey = m_deleteKey->currentText();
    return config;
}

void ServerConfigOptDlg::resetCurServer(ServerConfig *config)
{
    // qDebug() << "Enter ServerConfigOptDlg::resetCurServer";
    m_curServer = config;
}

void ServerConfigOptDlg::setAdvanceRegionVisible(bool isVisible)
{
    qCDebug(remotemanage) << "setAdvanceRegionVisible:" << isVisible;
    // 点击【高级选项】展开，优先隐藏选项，防止因布局变更导致的闪烁问题。
    bool focustOnAdvanced = m_advancedOptions->hasFocus();
    if (isVisible) {
        qCDebug(remotemanage) << "making advanced options visible";
        //切换【高级选项】的焦点
        m_advancedOptions->hide();
    } else {
        qCDebug(remotemanage) << "hiding advanced options";
        m_advancedOptions->show();
    }

    //隐藏或显示【高级选项】下方的控件
    int hideRow = m_pGridLayout->rowCount();
    for(int row = 0; row < m_pGridLayout->rowCount(); row ++) {
        for(int column = 0; column < m_pGridLayout->columnCount(); column ++) {
            QLayoutItem *item = m_pGridLayout->itemAtPosition(row, column);
            QWidget *cell = item ? item->widget() : nullptr;
            if(nullptr == cell)
                continue;

            //获取【高级选项】对应的行
            if(cell == m_advancedOptions)
                hideRow = row;
            //【高级选项】、【删除服务器】另外处理
            if(cell == m_advancedOptions
                    || cell == m_delServer)
                continue;
            //隐藏或显示【高级选项】下方的控件
            if(row > hideRow)
                cell->setVisible(isVisible);
        }
    }

    // 继续展开删除选项，重设界面大小
    if (isVisible) {
        qCDebug(remotemanage) << "expanding advanced options";
        if (focustOnAdvanced) {
            qCDebug(remotemanage) << "setting focus to group";
            m_group->setFocus();
        }

#ifdef DTKWIDGET_CLASS_DSizeMode
        //修改界面
        if (SCT_MODIFY == m_type) {
            qCDebug(remotemanage) << "modify type - showing delete server";
            m_delServer->show();
        } else {
            qCDebug(remotemanage) << "add type - hiding delete server";
            m_delServer->hide();
        }
        //singleShot是为了避免size和resize的不一样
        QTimer::singleShot(0, this, [=](){ resize(SETTING_DIALOG_WIDTH, sizeHint().height()); });

#else
        //修改界面
        if (SCT_MODIFY == m_type) {
            qCDebug(remotemanage) << "modify type - showing delete server (no DSizeMode)";
            m_delServer->show();
            //singleShot是为了避免size和resize的不一样
            QTimer::singleShot(0, this, [=](){resize(459, 670);});
        } else {
            qCDebug(remotemanage) << "add type - hiding delete server (no DSizeMode)";
            m_delServer->hide();
            QTimer::singleShot(0, this, [=](){resize(459, 630);});
        }
#endif
    } else {
        qCDebug(remotemanage) << "collapsing advanced options";
        m_delServer->hide();

#ifdef DTKWIDGET_CLASS_DSizeMode
        resize(SETTING_DIALOG_WIDTH, sizeHint().height());
#else
        resize(459, 392);    
#endif
    }
}

void ServerConfigOptDlg::slotAddSaveButtonClicked()
{
    qCInfo(remotemanage) << "ServerConfigOptDlg add and save button clicled slot function.";
    // 服务器名为空
    if (m_serverName->text().trimmed().isEmpty()) {
        qCDebug(remotemanage) << "server name is empty";
        m_serverName->showAlertMessage(tr("Please enter a server name"), m_serverName);
        return;
    }

    /***add begin by ut001121 zhangmeng 20200615 限制名称字符长度 修复BUG31286***/
    if (m_serverName->text().length() > MAX_NAME_LEN) {
        qCDebug(remotemanage) << "server name too long";
        m_serverName->showAlertMessage(QObject::tr("The name should be no more than 32 characters"), m_serverName);
        return;
    }
    /***add end by ut001121***/

    // 地址为空
    if (m_address->text().trimmed().isEmpty()) {
        qCDebug(remotemanage) << "address is empty";
        m_address->showAlertMessage(tr("Please enter an IP address"), m_address);
        return;
    }
    // 端口为空
    if (m_port->text().trimmed().isEmpty()) {
        qCDebug(remotemanage) << "port is empty";
        m_port_tip->showAlertMessage(tr("Please enter a port"));
        return;
    }

    //---added by qinyaning(nyq) to show the tip when username is empty---//
    if (m_userName->text().trimmed().isEmpty()) { //如果用户名为空， 提示用户名为空， 添加失败
        qCDebug(remotemanage) << "username is empty";
        m_userName->showAlertMessage(tr("Please enter a username"), m_userName);
        return;
    }
    m_currentServerName = m_serverName->text();
    //------------------------------------------------------------------//
    //--added by qinyaning(nyq) to solve the bug 19116: You can create a new remote server with the same name--//
    if ((SCT_ADD == m_type)
            || ((SCT_MODIFY == m_type  && m_curServer != nullptr)
                && (m_curServer->m_serverName.trimmed() != m_serverName->text().trimmed()))) { /*此时用户已经在修改模式下修改了服务器名称*/
        qCDebug(remotemanage) << "checking for duplicate server name";
        QMap<QString, QList<ServerConfig *>> severConfigs = ServerConfigManager::instance()->getServerConfigs();
        for (QMap<QString, QList<ServerConfig *>>::iterator iter = severConfigs.begin(); iter != severConfigs.end(); ++iter) {
            QList<ServerConfig *> value = iter.value();
            for (int i = 0; i < value.size(); i++) {
                if (value[i]->m_serverName.trimmed() == m_serverName->text().trimmed()) { //服务器名相同
                    qCDebug(remotemanage) << "duplicate server name found";
                    QString strFirstLine = tr("The server name already exists,");
                    QString strSecondeLine = tr("please input another one. ");
                    Utils::showSameNameDialog(this, strFirstLine, strSecondeLine);
                    return;
                }
            }
        }
    }
    //-------------------------------------------------------------------//
    ServerConfig *config = new ServerConfig();
    config->m_serverName = m_serverName->text().trimmed();
    config->m_address = m_address->text().trimmed();
    config->m_userName = m_userName->text().trimmed();
    config->m_password = m_password->text();
    config->m_privateKey = m_privateKey->text();
    config->m_port = m_port->text();
    config->m_group = m_group->currentText().trimmed();
    config->m_path = m_path->text();
    config->m_command = m_command->text();
    config->m_encoding = m_coding->currentText();
    config->m_backspaceKey = m_backSapceKey->currentText();
    config->m_deleteKey = m_deleteKey->currentText();
    if (SCT_ADD == m_type) {
        qCDebug(remotemanage) << "adding new server config";
        ServerConfigManager::instance()->saveServerConfig(config);
        ServerConfigManager::instance()->refreshList();
    } else if (SCT_MODIFY == m_type && m_curServer != nullptr) {
        qCDebug(remotemanage) << "modifying existing server config";
        ServerConfigManager::instance()->modifyServerConfig(config, m_curServer);
    }
    accept();
    qCInfo(remotemanage) << "Add or save remote config finish";
}

void ServerConfigOptDlg::slotFileChooseDialog()
{
    // qDebug() << "Enter ServerConfigOptDlg::slotFileChooseDialog";
    DFileDialog dialog(this, QObject::tr("Select the private key file"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden);
    dialog.setLabelText(QFileDialog::Accept, QObject::tr("Select"));

    int code = dialog.exec();

    if (QDialog::Accepted == code && !dialog.selectedFiles().isEmpty()) {
        qCDebug(remotemanage) << "file dialog accepted with selected files";
        QStringList list = dialog.selectedFiles();
        const QString fileName = list.first();

        m_privateKey->setText(fileName);
    }
}
