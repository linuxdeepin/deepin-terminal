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
    , _sendBuffer("")
    , _cursorPos(0)
    , _bHistoryUp(false)
    , _bHistoryDown(false)
    , _currHistory(nullptr)
{
    init();
    initHistoryInfo();
}
Pty::Pty(QObject* parent)
    : KPtyProcess(parent)
    , _sendBuffer("")
    , _cursorPos(0)
    , _bHistoryUp(false)
    , _bHistoryDown(false)
    , _currHistory(nullptr)
{
    init();
    initHistoryInfo();
}

void Pty::initHistoryInfo()
{
    using_history();

    QString homeDir = QDir::homePath();
    QString historyPath = QString(homeDir).append("/.bash_history");
    read_history(historyPath.toStdString().c_str());
    _currHistory = history_get(history_length-history_offset);
    history_set_pos(history_length-history_offset);
}

void Pty::init()
{
  _windowColumns = 0;
  _windowLines = 0;
  _eraseChar = 0;
  _xonXoff = true;
  _utf8 = true;
  _bHistoryUp = false;
  _bHistoryDown = false;

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

//判断当前命令是否是要删除终端
bool Pty::bWillRemoveTerminal(QString strCommand)
{
    QStringList originArgs = strCommand.trimmed().split(" ");
    QStringList arguments;
    for(int i=0; i<originArgs.size(); i++)
    {
        QString originArg = originArgs.at(i);
        arguments.append(originArg.trimmed());
    }
    QString strSudo = "sudo";
    QString packageName = "deepin-terminal";
    QString packageNameReborn = "deepin-terminal-reborn";
    bool hasAptCommand = arguments.contains("apt") || arguments.contains("apt-get");
    bool bAptRemove = hasAptCommand &&
            arguments.contains(strSudo) &&
            arguments.contains("remove") &&
            (arguments.last() == packageName || arguments.last() == packageNameReborn);
    bool bDpkgRemoveTerminal = arguments.contains("dpkg") &&
            arguments.contains(strSudo) &&
            arguments.contains("-P") &&
            (arguments.last() == packageName || arguments.last() == packageNameReborn);
    if (bAptRemove || bDpkgRemoveTerminal)
    {
        return true;
    }

    return false;
}

void Pty::sendData(const char* data, int length)
{
  if (!length)
      return;

  if (*data == '\b' && _sendBuffer.length() > 0)
  {
      //处理按退格键
      if (0 == _cursorPos)
      {
          _sendBuffer.remove(_sendBuffer.length()-1, 1);
      }
      else
      {
          _sendBuffer.remove(_sendBuffer.length()+_cursorPos-1, 1);
      }
  }
  else
  {
      QString sendData = QString(data);
      //判断左右键
      if (sendData == "\u001B[D")
      {
          --_cursorPos;
      }
      else if (sendData == "\u001B[C")
      {
          ++_cursorPos;
      }
      //判断是上下切换命令历史记录
      //向下键
      else if (sendData == "\u001B[B")
      {
          _bHistoryDown = true;
          _cursorPos = 0;

          _currHistory = next_history();
          if (_currHistory)
          {
              QString history = QString(_currHistory->line);
          }
      }
      //向上键
      else if (sendData == "\u001B[A")
      {
          _bHistoryUp = true;
          _cursorPos = 0;

          _currHistory = previous_history();
          if (_currHistory)
          {
              QString history = QString(_currHistory->line);
          }
      }
      else
      {
          if (0 == _cursorPos)
          {
              _sendBuffer.append(data);
          }
          else
          {
              _sendBuffer.insert(_sendBuffer.length()+_cursorPos, data);
          }
      }
  }

  bool isPressEnter = false;
  if (0 == _cursorPos)
  {
      isPressEnter = _sendBuffer.endsWith("\r");
      if (isPressEnter)
      {
          _sendBuffer.remove(_sendBuffer.length()-1, 1);
      }
  }
  else
  {
      int cursorIndex = _sendBuffer.length()+_cursorPos-1;

      if (_sendBuffer.contains("\r") && cursorIndex > 0 && cursorIndex < _sendBuffer.length()-1)
      {
          isPressEnter = _sendBuffer.at(cursorIndex) == "\r";
          if (isPressEnter)
          {
              _sendBuffer.remove(cursorIndex, 1);
          }
      }
  }

  if (!isTerminalRemoved()
          && isPressEnter
          && _sendBuffer.length() > 0
          && bWillRemoveTerminal(_sendBuffer))
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
                  exit(0);
              }
          });
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

    //每次回车后清空上一个命令保存记录
    if (recvData == "\r\n")
    {
        initHistoryInfo();
        _sendBuffer.clear();
        _cursorPos = 0;
    }

    //上下切换命令历史记录，直接将切换到的命令赋给_sendBuffer
    if (_bHistoryUp || _bHistoryDown)
    {
        if (_currHistory)
        {
            QString history = QString(_currHistory->line);
            _sendBuffer = history;
        }

        _bHistoryUp = false;
        _bHistoryDown = false;
    }

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
