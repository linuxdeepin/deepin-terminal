/*
 * This file is a part of QTerminal - http://gitorious.org/qterminal
 *
 * This file was un-linked from KDE and modified
 * by Maxim Bourmistrov <maxim@unixconn.com>
 *
 */

/*
    This file is part of Konsole, an X terminal.
    Copyright 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

// Own
#include "Pty.h"
#include "SessionManager.h"

// System
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <termios.h>
#include <csignal>

// Qt
#include <QStringList>
#include <QtDebug>
#include <QMessageBox>
#include <QDir>
#include <QRegExp>
#include <QRegExpValidator>
#include <QTimer>
#include <QMetaEnum>

#include "kpty.h"
#include "kptydevice.h"

using namespace Konsole;

void Pty::setWindowSize(int lines, int cols)
{


   // if((_windowColumns == cols) && (_windowLines == lines) && !lastCommandStateIsResize)
    //{
    //    return;
    //}
    lastSend = "";
    _windowColumns = cols;
    _windowLines = lines;
    if(!hasStart)
    {
        return;
    }

    lastCommandStateIsResize = true;
    qDebug()<<"update lastCommandStateIsResize"<<lastCommandStateIsResize;
    emit shellHasStart();

    qDebug()<<"setWindowSize"<<lines<<cols;
    if (pty()->masterFd() >= 0)
    {
        pty()->setWinSize(lines, cols);
    }

}
QSize Pty::windowSize() const
{
    return {_windowColumns, _windowLines};
}

void Pty::setFlowControlEnabled(bool enable)
{
    _xonXoff = enable;

    if (pty()->masterFd() >= 0) {
        struct ::termios ttmode;
        pty()->tcGetAttr(&ttmode);
        if (!enable)
            ttmode.c_iflag &= ~(IXOFF | IXON);
        else
            ttmode.c_iflag |= (IXOFF | IXON);
        if (!pty()->tcSetAttr(&ttmode))
            qWarning() << "Unable to set terminal attributes.";
    }
}
bool Pty::flowControlEnabled() const
{
    if (pty()->masterFd() >= 0) {
        struct ::termios ttmode;
        pty()->tcGetAttr(&ttmode);
        return ttmode.c_iflag & IXOFF &&
               ttmode.c_iflag & IXON;
    }
    qWarning() << "Unable to get flow control status, terminal not connected.";
    return false;
}

void Pty::setUtf8Mode(bool enable)
{
#ifdef IUTF8 // XXX not a reasonable place to check it.
    _utf8 = enable;

    if (pty()->masterFd() >= 0) {
        struct ::termios ttmode;
        pty()->tcGetAttr(&ttmode);
        if (!enable)
            ttmode.c_iflag &= ~IUTF8;
        else
            ttmode.c_iflag |= IUTF8;
        if (!pty()->tcSetAttr(&ttmode))
            qWarning() << "Unable to set terminal attributes.";
    }
#endif
}

void Pty::setErase(char erase)
{
    _eraseChar = erase;

    if (pty()->masterFd() >= 0) {
        struct ::termios ttmode;
        pty()->tcGetAttr(&ttmode);
        ttmode.c_cc[VERASE] = erase;
        if (!pty()->tcSetAttr(&ttmode))
            qWarning() << "Unable to set terminal attributes.";
    }
}

char Pty::erase() const
{
    if (pty()->masterFd() >= 0) {
        struct ::termios ttyAttributes;
        pty()->tcGetAttr(&ttyAttributes);
        return ttyAttributes.c_cc[VERASE];
    }

    return _eraseChar;
}

void Pty::addEnvironmentVariables(const QStringList &environment)
{
    QListIterator<QString> iter(environment);
    while (iter.hasNext()) {
        QString pair = iter.next();

        // split on the first '=' character
        int pos = pair.indexOf(QLatin1Char('='));

        if (pos >= 0) {
            QString variable = pair.left(pos);
            QString value = pair.mid(pos + 1);

            setEnv(variable, value);
        }
    }
}

int Pty::start(const QString &program,
               const QStringList &programArguments,
               const QStringList &environment,
               ulong winid,
               bool addToUtmp
               //const QString& dbusService,
               //const QString& dbusSession
              )
{
    clearProgram();

    // For historical reasons, the first argument in programArguments is the
    // name of the program to execute, so create a list consisting of all
    // but the first argument to pass to setProgram()
    Q_ASSERT(programArguments.count() >= 1);
    setProgram(program, programArguments.mid(1));

    addEnvironmentVariables(environment);

    setEnv(QLatin1String("WINDOWID"), QString::number(winid));
    setEnv(QLatin1String("COLORTERM"), QLatin1String("truecolor"));

    // unless the LANGUAGE environment variable has been set explicitly
    // set it to a null string
    // this fixes the problem where KCatalog sets the LANGUAGE environment
    // variable during the application's startup to something which
    // differs from LANG,LC_* etc. and causes programs run from
    // the terminal to display messages in the wrong language
    //
    // this can happen if LANG contains a language which KDE
    // does not have a translation for
    //
    // BR:149300
    setEnv(QLatin1String("LANGUAGE"), QString(), false /* do not overwrite existing value if any */);

    setUseUtmp(addToUtmp);

    struct ::termios ttmode;
    pty()->tcGetAttr(&ttmode);
    if (!_xonXoff)
        ttmode.c_iflag &= ~(IXOFF | IXON);
    else
        ttmode.c_iflag |= (IXOFF | IXON);
#ifdef IUTF8 // XXX not a reasonable place to check it.
    if (!_utf8)
        ttmode.c_iflag &= ~IUTF8;
    else
        ttmode.c_iflag |= IUTF8;
#endif

    if (_eraseChar != 0)
        ttmode.c_cc[VERASE] = _eraseChar;

    if (!pty()->tcSetAttr(&ttmode))
        qWarning() << "Unable to set terminal attributes.";

    //pty()->setWinSize(_windowLines, _windowColumns);


    KProcess::start();

    if (!waitForStarted())
        return -1;

    return 0;
}

void Pty::setEmptyPTYProperties()
{
    struct ::termios ttmode;
    pty()->tcGetAttr(&ttmode);
    if (!_xonXoff)
        ttmode.c_iflag &= ~(IXOFF | IXON);
    else
        ttmode.c_iflag |= (IXOFF | IXON);
#ifdef IUTF8 // XXX not a reasonable place to check it.
    if (!_utf8)
        ttmode.c_iflag &= ~IUTF8;
    else
        ttmode.c_iflag |= IUTF8;
#endif

    if (_eraseChar != 0)
        ttmode.c_cc[VERASE] = _eraseChar;

    if (!pty()->tcSetAttr(&ttmode))
        qWarning() << "Unable to set terminal attributes.";
}

void Pty::setWriteable(bool writeable)
{
    struct stat sbuf;
    stat(pty()->ttyName(), &sbuf);
    if (writeable)
        chmod(pty()->ttyName(), sbuf.st_mode | S_IWGRP);
    else
        chmod(pty()->ttyName(), sbuf.st_mode & ~(S_IWGRP | S_IWOTH));
}

Pty::Pty(int masterFd, QObject *parent)
    : KPtyProcess(masterFd, parent)
{
    init();
}

Pty::Pty(QObject *parent)
    : KPtyProcess(parent)
{
    init();
}

void Pty::init()
{
    _windowColumns = 0;
    _windowLines = 0;
    _eraseChar = 0;
    _xonXoff = true;
    _utf8 = true;
    _bUninstall = false;

    connect(pty(), SIGNAL(readyRead()), this, SLOT(dataReceived()));
    setPtyChannels(KPtyProcess::AllChannels);
}

Pty::~Pty()
{
}

void Pty::entryCustomFixStep(CustomFixStep step)
{
    qDebug()<<"entry"<<QMetaEnum::fromType<CustomFixStep>().key(step);
    m_CustomFixStep = step;
    emit customFixStepChanged(step);
}

void Pty::deleteReturnChar(QByteArray &data)
{
    if(data.startsWith("\r\n") )
    {
        qDebug()<<"deleteReturnChar"<<data;
        data = data.right(data.size()-2);
    }
    if(data.startsWith("\u0007\r\n") )
    {
        qDebug()<<"deleteReturnChar"<<data;
        data = data.right(data.size()-3);
    }
}

bool Pty::isTerminalRemoved()
{
    QFile terminalExecFile("/usr/bin/deepin-terminal");
    if (terminalExecFile.exists()) {
        return false;
    }

    return true;
}

bool isPatternAcceptable(QString strCommand, QString strPattern)
{
    QString strTrimmedCmd = strCommand.trimmed();

    QRegExp cmdRegExp;
    cmdRegExp.setPattern(strPattern);
    QRegExpValidator cmdREValidator(cmdRegExp, nullptr);

    int pos = 0;
    QValidator::State validateState = cmdREValidator.validate(strTrimmedCmd, pos);

    return (validateState == QValidator::Acceptable);
}

//判断当前命令是否是要删除终端
bool Pty::bWillRemoveTerminal(QString strCommand)
{
    QString packageName = "deepin-terminal";

    QStringList strCommandList;
    strCommandList.append(strCommand);

    if (strCommand.contains("&&")) {
        QStringList cmdList = strCommand.split("&&");
        for (int i = 0; i < cmdList.size(); i++) {
            QString currCmd = cmdList.at(i).trimmed();
            if (currCmd.length() > 0 && currCmd.contains(packageName)) {
                strCommandList.append(currCmd);
            }
        }
    }

    if (strCommand.contains(";")) {
        QStringList cmdList = strCommand.split(";");
        for (int i = 0; i < cmdList.size(); i++) {
            QString currCmd = cmdList.at(i).trimmed();
            if (currCmd.length() > 0 && currCmd.contains(packageName)) {
                strCommandList.append(currCmd);
            }
        }
    }

    QList<bool> acceptableList;

    QStringList packageNameList;
    packageNameList << packageName;

    for (int i = 0; i < strCommandList.size(); i++) {
        QString strCurrCommand = strCommandList.at(i);
        for (int j = 0; j < packageNameList.size(); j++) {
            QString packageName = packageNameList.at(j);
            QString removePattern = QString("sudo\\s+apt-get\\s+remove\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt\\s+remove\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);
            /******** Modify by nt001000 renfeixiang 2020-05-27:修改 放到bWillPurgeTerminal函数中 Begin***************/
//            removePattern = QString("sudo\\s+dpkg\\s+-P\\s+%1").arg(packageName);
//            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);
            /******** Modify by nt001000 renfeixiang 2020-05-27:修改 放到bWillPurgeTerminal函数中 End***************/

            removePattern = QString("sudo\\s+dpkg\\s+-r\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+rm\\s+.+\\s+/usr/bin/deepin-terminal");
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+rm\\s+/usr/bin/deepin-terminal");
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);
        }
    }

    return acceptableList.contains(true);
}

/******** Add by nt001000 renfeixiang 2020-05-27:增加 Purge卸载命令的判断，显示不同的卸载提示框 Begin***************/
bool Pty::bWillPurgeTerminal(QString strCommand)
{
    QString packageName = "deepin-terminal";

    QStringList strCommandList;
    strCommandList.append(strCommand);

    if (strCommand.contains("&&")) {
        QStringList cmdList = strCommand.split("&&");
        for (int i = 0; i < cmdList.size(); i++) {
            QString currCmd = cmdList.at(i).trimmed();
            if (currCmd.length() > 0 && currCmd.contains(packageName)) {
                strCommandList.append(currCmd);
            }
        }
    }

    if (strCommand.contains(";")) {
        QStringList cmdList = strCommand.split(";");
        for (int i = 0; i < cmdList.size(); i++) {
            QString currCmd = cmdList.at(i).trimmed();
            if (currCmd.length() > 0 && currCmd.contains(packageName)) {
                strCommandList.append(currCmd);
            }
        }
    }

    QList<bool> acceptableList;

    QStringList packageNameList;
    packageNameList << packageName;

    for (int i = 0; i < strCommandList.size(); i++) {
        QString strCurrCommand = strCommandList.at(i);
        for (int j = 0; j < packageNameList.size(); j++) {
            QString packageName = packageNameList.at(j);
            QString removePattern = QString("sudo\\s+apt-get\\s+purge\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt-get\\s+purge\\s+-y\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt-get\\s+remove\\s+--purge\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt-get\\s+--purge\\s+remove\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt\\s+purge\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt\\s+purge\\s+-y\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt\\s+remove\\s+--purge\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+apt\\s+--purge\\s+remove\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);

            removePattern = QString("sudo\\s+dpkg\\s+-P\\s+%1").arg(packageName);
            acceptableList << isPatternAcceptable(strCurrCommand, removePattern);
        }
    }

    return acceptableList.contains(true);
}
/******** Add by nt001000 renfeixiang 2020-05-27:增加 Purge卸载命令的判断，显示不同的卸载提示框 End***************/

void Pty::sendData(const char *data, int length)
{
    if (!length) {
        return;
    }

    bool isCustomCommand = false;
    QString currCommand = QString::fromLatin1(data);
    if (currCommand.length() > 0 && currCommand.endsWith('\n')) {
        isCustomCommand = true;
    }
    lastSend = currCommand;
    qDebug()<<"lastSend"<<lastSend;

    //检测到按了回车键
    if (((*data) == '\r' || isCustomCommand) && _bUninstall == false) {
        QString strCurrCommand = SessionManager::instance()->getCurrShellCommand(_sessionId);
        if (isCustomCommand) {
            strCurrCommand = currCommand;
        }

        /******** Modify by nt001000 renfeixiang 2020-05-27:修改 根据remove和purge卸载命令，发送信号不同参数值 Begin***************/
        bool bPurgeTerminal =  bWillPurgeTerminal(strCurrCommand);
        bool bRemoveTerminal =  bWillRemoveTerminal(strCurrCommand);

        if (!isTerminalRemoved() && (bPurgeTerminal || bRemoveTerminal)) {
            QString strname = "remove";
            if (bPurgeTerminal) {
                strname = "purge";
            }
            QMetaObject::invokeMethod(this, "ptyUninstallTerminal", Qt::AutoConnection, Q_RETURN_ARG(bool, _bUninstall), Q_ARG(QString, strname));
            /******** Modify by nt001000 renfeixiang 2020-05-27:修改 根据remove和purge卸载命令，发送信号不同参数值 End***************/
            if (_bUninstall) {
                qDebug() << "确认卸载终端！" << _bUninstall << endl;
                connect(SessionManager::instance(), &SessionManager::sessionIdle, this, [ = ](bool isIdle) {
                    //卸载完成，关闭所有终端窗口
                    if (isIdle) {
                        if (isTerminalRemoved()) {
                            pclose(popen("killall deepin-terminal", "r"));
                        }
                    }

                    if (!isTerminalRemoved()) {
                        _bUninstall = false;
                    }
                });
            } else {
                qDebug() << "不卸载终端！" << _bUninstall << endl;
                return;
            }
        }
    }
//    lastCommandStateIsResize = false;
//    if (!pty()->write(data, length)) {
//        qWarning() << "Pty::doSendJobs - Could not send input data to terminal process.";
//        return;
//    }
//    return;


    qDebug()<<lastCommandStateIsResize;
    if(lastCommandStateIsResize /*&& hasStart*/)
    {
        //qDebug()<<"now start byteCtrlU";
        entryCustomFixStep(FixStep1_Ctrl_u);
        //Step1_Ctrl_u = true;

        QByteArray byteCtrlU("\u0015");
        if (!pty()->write(byteCtrlU, byteCtrlU.count())) {
            qWarning() << "Pty::doSendJobs - Could not send input data to terminal process.";
            return;
        }
        //return;

        //modifyByte

        QTimer::singleShot(5, this, [this]() {
            //entryCustomFixStep(FixStep2_Clear);
            m_userKey.append(lastSend);
            //qDebug()<<"now start clear";
            //Step2_Clear = true;
                char str1[] = "\x0d\x1b\x5b\x4b";
                char *strAdd = "\x1b\x5b\x41\u0007";
                QByteArray byteClear(str1);
                byteClear.append(strAdd);
                emit receivedData(byteClear.constData(), byteClear.count());


                QTimer::singleShot(2, this, [this]() {
                    entryCustomFixStep(FixStep3_Return);
                    QByteArray byteReturn("\r");
                    lastSend = byteReturn;
                    if (!pty()->write(byteReturn, byteReturn.count())) {
                        qWarning() << "Pty::doSendJobs - Could not send input data to terminal process.";
                        return;
                    }

                    QTimer::singleShot(2, this, [this]() {
//                        if(swapByte.isEmpty())
//                        {
//                            entryCustomFixStep(FixStep6_Complete);
//                        }
                        entryCustomFixStep(FixStep4_SwapText);
                        QByteArray byteReturn;
                        byteReturn.append(swapByte/*.replace(' ', "")*/);
                        lastSend = byteReturn;
                        if(!lastSend.isEmpty())
                        {
                            if (!pty()->write(byteReturn, byteReturn.count())) {
                                qWarning() << "Pty::doSendJobs - Could not send input data to terminal process.";
                                return;
                            }
                            swapByte.clear();
                        }

                        QTimer::singleShot(2, this, [this]() {
                            entryCustomFixStep(FixStep5_UserKey);
                            qDebug()<<"now start userkey"<<m_userKey;
                            if(!m_userKey.isEmpty())
                            {
                                if (!pty()->write(m_userKey, m_userKey.count())) {
                                    qWarning() << "Pty::doSendJobs - Could not send input data to terminal process.";
                                    return;
                                }
                                m_userKey.clear();
                            }
                        });

                    });
                 });


        });
        return;

        //lastCommandStateIsResize = false;

    }
//    if(!hasStart)
//    {
//        hasStart = true;
//    }
//    if(lastSend == "\r")
//    {
//        lastCommandStateIsResize = false;
//    }
    if (!pty()->write(data, length)) {
        qWarning() << "Pty::doSendJobs - Could not send input data to terminal process.";
        return;
    }
}

void Pty::dataReceived()
{
    QByteArray data = pty()->readAll();

    QString recvData = QString(data);

    switch (m_CustomFixStep) {
    case FixStep1_Ctrl_u:
        qDebug()<<"Step1_Ctrl_u = true, ignore dataReceived" <<recvData;

        return;
    //case FixStep2_Clear:
       // deleteReturnChar(data);
        break;
    case FixStep3_Return:
        deleteReturnChar(data);
        break;
//    case FixStep1_Ctrl_u:
//        qDebug()<<"Step1_Ctrl_u = true, ignore dataReceived" <<recvData;
//        return;
//    case FixStep1_Ctrl_u:
//        qDebug()<<"Step1_Ctrl_u = true, ignore dataReceived" <<recvData;
//        return;
    default:
        break;
    }

    if(data.isEmpty())
    {
        return;
    }


    //qDebug() << "____________________recv:" << recvData;
//    /******** Modify by m000714 daizhengwen 2020-04-30: 处理上传下载时乱码显示命令不执行****************/
//    // 乱码提示信息不显示
//    if (recvData.contains("bash: $'\\212")
//            || recvData.contains("bash: **0800000000022d：")
//            || recvData.contains("**^XB0800000000022d")
//            || recvData.startsWith("**\u0018B0800000000022d\r\u008A")) {
//        return;
//    }

//    // "\u008A"这个乱码不替换调会导致显示时有\b的效果导致命令错乱bug#23741
//    if (recvData.contains("\u008A")) {
//        recvData.replace("\u008A", "\b \b #");
//        data = recvData.toUtf8();
//    }

//    if (recvData == "rz waiting to receive.") {
//        recvData += "\r\n";
//        data = recvData.toUtf8();
//    }
    /********************* Modify by m000714 daizhengwen End ************************/


    emit receivedData(data.constData(), data.count());
    if(!hasStart)
    {
        hasStart = true;
        setWindowSize(_windowLines, _windowColumns);
        //emit shellHasStart();

    }

//     = cols;
//     = lines;
}

void Pty::lockPty(bool lock)
{
    Q_UNUSED(lock);

// TODO: Support for locking the Pty
    //if (lock)
    //suspend();
    //else
    //resume();
}

int Pty::foregroundProcessGroup() const
{
    int pid = tcgetpgrp(pty()->masterFd());

    if (pid != -1) {
        return pid;
    }

    return 0;
}

void Pty::setSessionId(int sessionId)
{
    _sessionId = sessionId;
}

void Pty::setupChildProcess()
{
    KPtyProcess::setupChildProcess();

    // reset all signal handlers
    // this ensures that terminal applications respond to
    // signals generated via key sequences such as Ctrl+C
    // (which sends SIGINT)
    struct sigaction action;
    sigset_t sigset;
    sigemptyset(&action.sa_mask);
    sigemptyset(&sigset);
    action.sa_handler = SIG_DFL;
    action.sa_flags = 0;
    for (int signal = 1; signal < NSIG; signal++) {
        sigaction(signal, &action, nullptr);
        sigaddset(&sigset, signal);
    }
    sigprocmask(SIG_UNBLOCK, &sigset, nullptr);
}
