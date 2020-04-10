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
#include <errno.h>
#include <termios.h>
#include <signal.h>

// Qt
#include <QStringList>
#include <QtDebug>
#include <QMessageBox>
#include <QDir>
#include <QRegExp>
#include <QRegExpValidator>

#include "kpty.h"
#include "kptydevice.h"

using namespace Konsole;

void Pty::setWindowSize(int lines, int cols)
{
  _windowColumns = cols;
  _windowLines = lines;

  if (pty()->masterFd() >= 0)
    pty()->setWinSize(lines, cols);
}
QSize Pty::windowSize() const
{
    return QSize(_windowColumns,_windowLines);
}

void Pty::setFlowControlEnabled(bool enable)
{
  _xonXoff = enable;

  if (pty()->masterFd() >= 0)
  {
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
    if (pty()->masterFd() >= 0)
    {
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

  if (pty()->masterFd() >= 0)
  {
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

  if (pty()->masterFd() >= 0)
  {
    struct ::termios ttmode;
    pty()->tcGetAttr(&ttmode);
    ttmode.c_cc[VERASE] = erase;
    if (!pty()->tcSetAttr(&ttmode))
      qWarning() << "Unable to set terminal attributes.";
  }
}

char Pty::erase() const
{
    if (pty()->masterFd() >= 0)
    {
        struct ::termios ttyAttributes;
        pty()->tcGetAttr(&ttyAttributes);
        return ttyAttributes.c_cc[VERASE];
    }

    return _eraseChar;
}

void Pty::addEnvironmentVariables(const QStringList& environment)
{
    QListIterator<QString> iter(environment);
    while (iter.hasNext())
    {
        QString pair = iter.next();

        // split on the first '=' character
        int pos = pair.indexOf(QLatin1Char('='));

        if ( pos >= 0 )
        {
            QString variable = pair.left(pos);
            QString value = pair.mid(pos+1);

            setEnv(variable,value);
        }
    }
}

int Pty::start(const QString& program,
               const QStringList& programArguments,
               const QStringList& environment,
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
  setEnv(QLatin1String("LANGUAGE"),QString(),false /* do not overwrite existing value if any */);

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

  pty()->setWinSize(_windowLines, _windowColumns);

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
    chmod(pty()->ttyName(), sbuf.st_mode & ~(S_IWGRP|S_IWOTH));
}

Pty::Pty(int masterFd, QObject* parent)
    : KPtyProcess(masterFd,parent)
{
    init();
}

Pty::Pty(QObject* parent)
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

  connect(pty(), SIGNAL(readyRead()) , this , SLOT(dataReceived()));
  setPtyChannels(KPtyProcess::AllChannels);
}

Pty::~Pty()
{
}

bool Pty::isTerminalRemoved()
{
    QFile terminalExecFile("/usr/bin/deepin-terminal");
    if (terminalExecFile.exists())
    {
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
    QString packageNameReborn = "deepin-terminal-reborn";

    QList<bool> acceptableList;

    QStringList packageNameList;
    packageNameList << packageName << packageNameReborn;
    for(int i=0; i<packageNameList.size(); i++)
    {
        QString removePattern = QString("sudo\\s+apt-get\\s+remove\\s+%1").arg(packageNameList.at(i));
        acceptableList << isPatternAcceptable(strCommand, removePattern);

        removePattern = QString("sudo\\s+apt\\s+remove\\s+%1").arg(packageNameList.at(i));
        acceptableList << isPatternAcceptable(strCommand, removePattern);

        removePattern = QString("sudo\\s+dpkg\\s+-P\\s+%1").arg(packageNameList.at(i));
        acceptableList << isPatternAcceptable(strCommand, removePattern);

        removePattern = QString("sudo\\s+rm\\s+.+\\s+/usr/bin/deepin-terminal");
        acceptableList << isPatternAcceptable(strCommand, removePattern);

        removePattern = QString("sudo\\s+rm\\s+/usr/bin/deepin-terminal");
        acceptableList << isPatternAcceptable(strCommand, removePattern);
    }

    return acceptableList.contains(true);
}

void Pty::sendData(const char* data, int length)
{
    if (!length)
    {
        return;
    }

    //检测到按了回车键
    if((*data) == '\r')
    {
        QString strCurrCommand = SessionManager::instance()->getCurrShellCommand();
        if (!isTerminalRemoved() && bWillRemoveTerminal(strCurrCommand))
        {
            QMessageBox messageBox(QMessageBox::NoIcon,
                                   "警告", "您确定要卸载终端吗，卸载后将无法再使用终端应用?",
                                   QMessageBox::Yes | QMessageBox::No, nullptr);
            int result = messageBox.exec();
            if (QMessageBox::No == result)
            {
                return;
            }
            else
            {
                connect(SessionManager::instance(), &SessionManager::sessionIdle, this, [=](bool isIdle) {
                    if (isIdle && isTerminalRemoved())
                    {
                        pclose(popen("killall deepin-terminal", "r"));
                    }
                });
            }
        }
    }

    if (!pty()->write(data,length))
    {
        qWarning() << "Pty::doSendJobs - Could not send input data to terminal process.";
        return;
    }
}

void Pty::dataReceived()
{
    QByteArray data = pty()->readAll();

    QString recvData = QString(data);

    emit receivedData(data.constData(),data.count());
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

    if ( pid != -1 )
    {
        return pid;
    }

    return 0;
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
    action.sa_handler = SIG_DFL;
    action.sa_flags = 0;
    for (int signal=1;signal < NSIG; signal++) {
        sigaction(signal,&action,0L);
        sigaddset(&sigset, signal);
    }
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);
}
