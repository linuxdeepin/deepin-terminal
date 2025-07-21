// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "remotemanagementplugn.h"
#include "mainwindow.h"
#include "utils.h"
#include "service.h"
#include "termwidget.h"

#include <QTextCodec>
#include <QDebug>

Q_DECLARE_LOGGING_CATEGORY(remotemanage)

RemoteManagementPlugin::RemoteManagementPlugin(QObject *parent) : MainWindowPluginInterface(parent)
{
    qCDebug(remotemanage) << "RemoteManagementPlugin constructor";
    Utils::set_Object_Name(this);
    m_pluginName = "Remote Management";
}

void RemoteManagementPlugin::initPlugin(MainWindow *mainWindow)
{
    qCInfo(remotemanage) << "RemoteManagementPlugin init Plugin.";
    m_mainWindow = mainWindow;
    //initRemoteManagementTopPanel();
    connect(m_mainWindow, &MainWindow::showPluginChanged,  this, [ = ](const QString name, bool bSetFocus) {
        qCDebug(remotemanage) << "Lambda: showPluginChanged signal received";
        if (MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT != name) {
            qCDebug(remotemanage) << "Branch: plugin type is not remote management";
            // 判断窗口是否已经显示
            if (m_isShow) {
                qCDebug(remotemanage) << "Branch: window is shown";
                // 若显示，则隐藏
                RemoteManagementTopPanel * panel = getRemoteManagementTopPanel();
                if(panel) {
                    qCDebug(remotemanage) << "Branch: panel exists";
                    panel->hideAnim();
                }
                m_isShow = false;
            }

        } else {
            qCDebug(remotemanage) << "Branch: plugin type is remote management";
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，远程连接界面使用不方便，将雷神窗口变大适应正常的远程连接界面 Begin***************/
            if (m_mainWindow->isQuakeMode() && m_mainWindow->height() < LISTMINHEIGHT) {
                qCDebug(remotemanage) << "quake mode and height less than LISTMINHEIGHT";
                //因为拉伸函数设置了FixSize，导致自定义界面弹出时死循环，然后崩溃的问题
                QuakeWindow *quakeWindow = qobject_cast<QuakeWindow *>(m_mainWindow);
                if (nullptr == quakeWindow) {
                    qCDebug(remotemanage) << "quakeWindow is null, returning";
                    return;
                }
                quakeWindow->switchEnableResize(true);
                m_mainWindow->resize(m_mainWindow->width(), LISTMINHEIGHT); //首先设置雷神界面的大小
                m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);//重新打开远程连接界面，当前流程结束
                //窗口弹出后，重新判断雷神窗口是否需要有拉伸属性
                quakeWindow->switchEnableResize();
                return;
            }
            /******** Add by nt001000 renfeixiang 2020-05-18:修改雷神窗口太小时，远程连接界面使用不方便，将雷神窗口变大适应正常的远程连接界面 End***************/
            getRemoteManagementTopPanel()->show();
            m_isShow = true;
            if (bSetFocus) {
                qCDebug(remotemanage) << "setting focus in panel";
                // 若焦点不在终端上，焦点落入平面
                getRemoteManagementTopPanel()->setFocusInPanel();
            } else {
                qCDebug(remotemanage) << "setting focus on current page";
                // 若焦点在终端上，焦点落回终端
                m_mainWindow->focusCurrentPage();
            }
        }
    });
    connect(m_mainWindow, &MainWindow::quakeHidePlugin, this, [ = ]() {
        qCDebug(remotemanage) << "Lambda: quakeHidePlugin signal received";
        // 焦点在列表上，隐藏时，焦点现在当前窗口上
        if (m_mainWindow->isFocusOnList()) {
            qCDebug(remotemanage) << "focus is on list, setting focus on current page";
            m_mainWindow->focusCurrentPage();
            qCInfo(remotemanage) << "focus on remote list, hide remote list and set foucs on terminal";
        }
        getRemoteManagementTopPanel()->hide();
    });
}

QAction *RemoteManagementPlugin::titlebarMenu(MainWindow *mainWindow)
{
    qCDebug(remotemanage) << "Enter titlebarMenu";
    QAction *remoteManagementAction(new QAction(tr("Remote management"), mainWindow));

    connect(remoteManagementAction, &QAction::triggered, mainWindow, [mainWindow]() {
        qCDebug(remotemanage) << "Lambda: remoteManagementAction triggered";
        mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_REMOTEMANAGEMENT);
    });
    qCDebug(remotemanage) << "Exit titlebarMenu";
    return remoteManagementAction;
}

RemoteManagementTopPanel *RemoteManagementPlugin::getRemoteManagementTopPanel()
{
    qCDebug(remotemanage) << "Enter getRemoteManagementTopPanel";
    if (nullptr == m_remoteManagementTopPanel) {
        qCDebug(remotemanage) << "Initializing remote management top panel";
        initRemoteManagementTopPanel();
    }
    qCDebug(remotemanage) << "Exit getRemoteManagementTopPanel";
    return m_remoteManagementTopPanel;
}

void RemoteManagementPlugin::initRemoteManagementTopPanel()
{
    qCDebug(remotemanage) << "Enter initRemoteManagementTopPanel";
    m_remoteManagementTopPanel = new RemoteManagementTopPanel(m_mainWindow->centralWidget());
    m_remoteManagementTopPanel->setObjectName("RemoteManagementTopPanel");
    connect(m_remoteManagementTopPanel,
            &RemoteManagementTopPanel::doConnectServer,
            this,
            &RemoteManagementPlugin::doCennectServer, Qt::QueuedConnection);
}

void RemoteManagementPlugin::doCennectServer(ServerConfig *curServer)
{
    qCInfo(remotemanage) << "RemoteManagementPlugin do connect server.";
    if (nullptr != curServer) {
        qCDebug(remotemanage) << "curServer is not null";

        QString shellFile = createShellFile(curServer);
        QString strTxt = "expect -f " + shellFile + "\n";
        //--added by qinyaning(nyq) to solve the probelm which Connecting to the remote server
        /*does not connect to the remote server directly in the new TAB. time: 2020.4.13 18:15
         * */
        if (m_mainWindow->currentActivatedTerminal()->hasRunningProcess()) {
            qCDebug(remotemanage) << "terminal has running process, adding new tab";
            m_mainWindow->addTab(m_mainWindow->currentPage()->createCurrentTerminalProperties(), true);
        }

        //--------------------------------//
        /******** Modify by m000714 daizhengwen 2020-04-30: 将当前还没执行的命令清空****************/
        m_mainWindow->focusCurrentPage();
        m_mainWindow->pressCtrlU();
        m_mainWindow->sleep(100);
        /********************* Modify by m000714 daizhengwen End ************************/
        m_mainWindow->currentPage()->sendTextToCurrentTerm(strTxt, true);
        // 等待连接 100ms等待命令发过去正常立即执行，100ms足够，一下的信号槽只是判断是否开启另一个程序去连接
        // 若有程序去连接，则判断已连接，若连接失败，则判断为断开连接
        m_mainWindow->currentActivatedTerminal()->inputRemotePassword(curServer->m_password);
        QTimer::singleShot(100, this, [ = ]() {
            qCDebug(remotemanage) << "Lambda: QTimer::singleShot callback for connection check";
            TermWidget *term = m_mainWindow->currentActivatedTerminal();
            if (!term) {
                // 若term为空
                qCInfo(remotemanage) << "current terminal is null";
            }
            // 判断是否连接服务器
            if (!term->isInRemoteServer()) {
                // 没有连接上
                qCWarning(remotemanage) << "disconnect to server";
                return;
            }
            qCDebug(remotemanage) << "connected to server successfully";
            // 标记此term连接远程
            term->setIsConnectRemote(true);
            // 设置远程主机
            term->modifyRemoteTabTitle(*curServer);
            qCInfo(remotemanage) << "connect to server";
            // 编码
            setRemoteEncode(curServer->m_encoding);
            // 退格键
            setBackspaceKey(term, curServer->m_backspaceKey);
            // 删除键
            setDeleteKey(term, curServer->m_deleteKey);
        });
    }
    /******** Modify by ut000610 daizhengwen 2020-06-04: 点击连接服务器后，隐藏列表，焦点回到主窗口****************/
    m_mainWindow->showPlugin(MainWindow::PLUGIN_TYPE_NONE);
    // 隐藏列表后，将焦点设置到主窗口 100ms足够
    QTimer::singleShot(100, this, [&]() {
        qCDebug(remotemanage) << "Lambda: QTimer::singleShot callback for focus setting";
        if (m_mainWindow->isActiveWindow()) {
            qCDebug(remotemanage) << "main window is active, setting focus on current page";
            m_mainWindow->focusCurrentPage();
        }
    });
    /********************* Modify by ut000610 daizhengwen End ************************/
}

inline QString RemoteManagementPlugin::convertStringToAscii(const QString &strSrc)
{
    // qCDebug(remotemanage) << "Enter RemoteManagementPlugin::convertStringToAscii";
    QByteArray byte = strSrc.toLatin1();

    return QString(byte.toHex());
}

QString RemoteManagementPlugin::createShellFile(ServerConfig *curServer)
{
    qCInfo(remotemanage) << "RemoteManagementPlugin create temporary shell file.";
    // 首先读取通用模板
    QFile sourceFile(":/other/ssh_login.sh");
    QString fileString;
    // 打开文件
    if (sourceFile.open(QIODevice::ReadOnly)) {
        qCDebug(remotemanage) << "source file opened successfully";
        //读取文件
        fileString = sourceFile.readAll();
        // 关闭文件
        sourceFile.close();
    } else {
        qCDebug(remotemanage) << "failed to open source file";
    }

    QString strArgs = QString(" '<<USER>>' <<SERVER>> <<PORT>> '<<PRIVATE_KEY>>' '<<PASSWORD>>'");
    // 用远程管理数据替换文件内的关键字
    strArgs.replace("<<USER>>", curServer->m_userName);
    strArgs.replace("<<SERVER>>", curServer->m_address.trimmed());
    strArgs.replace("<<PORT>>", curServer->m_port);
    // 根据是否有证书，替换关键字 // pass_way仅仅是占位
    if (curServer->m_privateKey.isNull() || curServer->m_privateKey.isEmpty()) {
        qCDebug(remotemanage) << "no private key, using password authentication";
        fileString.replace("<<AUTHENTICATION>>", "no");
        strArgs.replace("<<PRIVATE_KEY>>", "NoPrivateKeyPath");
        // fix bug#64758 修改服务器密码，点击连接没有密码错误提示语，且可以成功连接
        // 由于密码中可能存在各种特殊符号如!、#、$等，不处理的话shell命令运行会报错，需要转成ASCII编码处理下
        QString asciiPassword = convertStringToAscii(curServer->m_password);
        strArgs.replace("<<PASSWORD>>", "");
    } else {
        qCDebug(remotemanage) << "using private key authentication";
        fileString.replace("<<AUTHENTICATION>>", "yes");
        strArgs.replace("<<PRIVATE_KEY>>", curServer->m_privateKey);
        strArgs.replace("<<PASSWORD>>", "");
    }

    QString path = curServer->m_path;
    QString command = curServer->m_command;
    // 添加远程提示
    QString remote_command = "echo " + tr("Make sure that rz and sz commands have been installed in the server before right clicking to upload and download files.") + " && ";
    if (!path.isNull() && !path.isEmpty()) {
        qCDebug(remotemanage) << "path is not empty, adding cd command";
        remote_command = remote_command + "cd " + path + " && ";
    }

    if (!command.isNull() && !command.isEmpty()) {
        qCDebug(remotemanage) << "command is not empty, adding custom command";
        remote_command = remote_command + command + " && ";
    }

    fileString.replace("<<REMOTE_COMMAND>>", remote_command);

    // 创建临时文件和执行脚本所需要的参数
    QString toFileStr = "/tmp/terminal-" + Utils::getRandString();
    QFile toFile(toFileStr);
    toFile.open(QIODevice::WriteOnly | QIODevice::Text);
    // 写文件
    toFile.write(fileString.toUtf8());
    // 退出文件
    toFile.close();
    return QString(toFileStr + strArgs);
}

void RemoteManagementPlugin::setRemoteEncode(QString encode)
{
    qCDebug(remotemanage) << "Enter setRemoteEncode, encode:" << encode;
    TermWidget *term = m_mainWindow->currentActivatedTerminal();
    if (!encode.isNull() && !encode.isEmpty()) {
        qCDebug(remotemanage) << "encode is not empty, setting text codec";
        // 设置当前窗口的编码
        term->setTextCodec(QTextCodec::codecForName(encode.toLocal8Bit()));
        qCInfo(remotemanage) << "Remote encode " << encode;
    }
    // 记录远程编码
    term->setRemoteEncode(encode);
    // 切换编码列表的编码
    emit Service::instance()->checkEncode(encode);
    qCDebug(remotemanage) << "Exit setRemoteEncode";
}

void RemoteManagementPlugin::setBackspaceKey(TermWidget *term, QString backspaceKey)
{
    qCDebug(remotemanage) << "Enter setBackspaceKey, key:" << backspaceKey;
    if ("control-h" == backspaceKey) {
        qCDebug(remotemanage) << "setting backspace mode to control-h";
        term->setBackspaceMode(EraseMode_Control_H);
    } else if ("auto" == backspaceKey) {
        qCDebug(remotemanage) << "setting backspace mode to auto";
        term->setBackspaceMode(EraseMode_Auto);
    } else if ("escape-sequence" == backspaceKey) {
        qCDebug(remotemanage) << "setting backspace mode to escape-sequence";
        term->setBackspaceMode(EraseMode_Escape_Sequeue);
    } else if ("ascii-del" == backspaceKey) {
        qCDebug(remotemanage) << "setting backspace mode to ascii-del";
        term->setBackspaceMode(EraseMode_Ascii_Delete);
    } else if ("tty" == backspaceKey) {
        qCDebug(remotemanage) << "setting backspace mode to tty";
        term->setBackspaceMode(EraseMode_TTY);
    }

    qCInfo(remotemanage) << "backspace mode " << backspaceKey;
}

void RemoteManagementPlugin::setDeleteKey(TermWidget *term, QString deleteKey)
{
    qCDebug(remotemanage) << "Enter setDeleteKey, key:" << deleteKey;
    if ("control-h" == deleteKey) {
        qCDebug(remotemanage) << "setting delete mode to control-h";
        term->setDeleteMode(EraseMode_Control_H);
    } else if ("auto" == deleteKey) {
        qCDebug(remotemanage) << "setting delete mode to auto";
        term->setDeleteMode(EraseMode_Auto);
    } else if ("escape-sequence" == deleteKey) {
        qCDebug(remotemanage) << "setting delete mode to escape-sequence";
        term->setDeleteMode(EraseMode_Escape_Sequeue);
    } else if ("ascii-del" == deleteKey) {
        qCDebug(remotemanage) << "setting delete mode to ascii-del";
        term->setDeleteMode(EraseMode_Ascii_Delete);
    } else if ("tty" == deleteKey) {
        qCDebug(remotemanage) << "setting delete mode to tty";
        term->setDeleteMode(EraseMode_TTY);
    }

    qCInfo(remotemanage) << "delete mode " << deleteKey;
}
