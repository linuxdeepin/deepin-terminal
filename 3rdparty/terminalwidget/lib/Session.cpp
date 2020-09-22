/*
    This file is part of Konsole

    Copyright (C) 2006-2007 by Robert Knight <robertknight@gmail.com>
    Copyright (C) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>, Copyright (C)2008

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
#include "Session.h"

// Standard
#include <cstdlib>

// Qt
#include <QApplication>
#include <QByteRef>
#include <QDir>
#include <QFile>
#include <QRegExp>
#include <QStringList>
#include <QFile>
#include <QtDebug>
//add by 2020-09-18
#include <QMetaEnum>

#include "Pty.h"
#include "ProcessInfo.h"
//#include "kptyprocess.h"
#include "TerminalDisplay.h"
#include "ShellCommand.h"
#include "Vt102Emulation.h"
//add by 2020-09-18
#include "TerminalCharacterDecoder.h"

using namespace Konsole;

int Session::lastSessionId = 0;

Session::Session(QObject *parent)
    : QObject(parent),
      _shellProcess(nullptr),
      _emulation(nullptr),
      _monitorActivity(false),
      _monitorSilence(false),
      _notifiedActivity(false),
      _autoClose(true),
      _wantedClose(false),
      _silenceSeconds(10),
      _isTitleChanged(false),
      _addToUtmp(false)  // disabled by default because of a bug encountered on certain systems
                         // which caused Konsole to hang when closing a tab and then opening a new
                         // one.  A 'QProcess destroyed while still running' warning was being
                         // printed to the terminal.  Likely a problem in KPty::logout()
                         // or KPty::login() which uses a QProcess to start /usr/bin/utempter
      ,
      _flowControl(true),
      _fullScripting(false),
      _sessionId(0),
      _sessionProcessInfo(nullptr),
      _foregroundProcessInfo(nullptr)
      //   , _zmodemBusy(false)
      //   , _zmodemProc(0)
      //   , _zmodemProgress(0)
      ,
      _hasDarkBackground(false)
{
    //prepare DBus communication
//    new SessionAdaptor(this);
    _sessionId = ++lastSessionId;
//    QDBusConnection::sessionBus().registerObject(QLatin1String("/Sessions/")+QString::number(_sessionId), this);

    //create teletype for I/O with shell process
    _shellProcess = new Pty();
    _shellProcess->setSessionId(_sessionId);
    //add by 2020-09-18
    _shellProcess->setRedrawStep(&m_RedrawStep);
    ptySlaveFd = _shellProcess->pty()->slaveFd();

    //create emulation backend
    _emulation = new Vt102Emulation();
    _emulation->setSessionId(_sessionId);

    connect( _emulation, SIGNAL( titleChanged( int, const QString & ) ),
             this, SLOT( setUserTitle( int, const QString & ) ) );
    connect( _emulation, SIGNAL( stateSet(int) ),
             this, SLOT( activityStateSet(int) ) );
//    connect( _emulation, SIGNAL( zmodemDetected() ), this ,
//            SLOT( fireZModemDetected() ) );
    connect( _emulation, SIGNAL( changeTabTextColorRequest( int ) ),
             this, SIGNAL( changeTabTextColorRequest( int ) ) );
    connect( _emulation, SIGNAL(profileChangeCommandReceived(const QString &)),
             this, SIGNAL( profileChangeCommandReceived(const QString &)) );

    connect(_emulation, SIGNAL(imageResizeRequest(QSize)),
            this, SLOT(onEmulationSizeChange(QSize)));
    connect(_emulation, SIGNAL(imageSizeChanged(int, int)),
            this, SLOT(onViewSizeChange(int, int)));
    connect(_emulation, &Vt102Emulation::cursorChanged,
            this, &Session::cursorChanged);

    //connect teletype to emulation backend
    _shellProcess->setUtf8Mode(_emulation->utf8());


    connect(_shellProcess, SIGNAL(receivedData(const char *, int)), this, SLOT(onReceiveBlock(const char *, int)));
    //add by 2020-09-18 begin
    connect(_shellProcess, &Pty::winsizeChanged,  this, [this](int lines, int columns){
        /******** Modify by ut000439 wangpeili 2020-09-03:修改不调用bash的功能，不走清行流程 Begin***************/
        _emulation->setImageSize(lines, columns);//不调用bash的流程也需要resizeImage，内容换行
        QString name = getDynamicProcessName();
        if(name == "bash")
        {
//            qDebug() << "bashbashbashbashbash";
            entryRedrawStep(RedrawStep1_Resize_Receiving);
            return ;
        }
        qDebug()<<"no need go to redraw mode, current process name:"<<name;
        /******** Modify by ut000439 wangpeili 2020-09-03 End***************/

    });
    //add by 2020-09-18 end

    connect(_emulation, SIGNAL(sendData(const char *, int, bool)), _shellProcess, SLOT(sendData(const char *, int, bool)));
    connect(_emulation, SIGNAL(lockPtyRequest(bool)), _shellProcess, SLOT(lockPty(bool)));
    connect(_emulation, SIGNAL(useUtf8Request(bool)), _shellProcess, SLOT(setUtf8Mode(bool)));

    connect(_shellProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(done(int)));
    // not in kprocess anymore connect( _shellProcess,SIGNAL(done(int)), this, SLOT(done(int)) );

    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/
    // 用于卸载终端弹出框提示
    connect( _shellProcess,SIGNAL(ptyUninstallTerminal(QString)), this, SIGNAL(sessionUninstallTerminal(QString)) );
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/

    //setup timer for monitoring session activity
    _monitorTimer = new QTimer(this);
    _monitorTimer->setSingleShot(true);
    connect(_monitorTimer, SIGNAL(timeout()), this, SLOT(monitorTimerDone()));
}

WId Session::windowId() const
{
    // On Qt5, requesting window IDs breaks QQuickWidget and the likes,
    // for example, see the following bug reports:
    // https://bugreports.qt.io/browse/QTBUG-40765
    // https://codereview.qt-project.org/#/c/94880/
    return 0;
}

void Session::setDarkBackground(bool darkBackground)
{
    _hasDarkBackground = darkBackground;
}
bool Session::hasDarkBackground() const
{
    return _hasDarkBackground;
}
bool Session::isRunning() const
{
    return _shellProcess->state() == QProcess::Running;
}

void Session::setCodec(QTextCodec * codec)
{
    emulation()->setCodec(codec);
}

void Session::setProgram(const QString & program)
{
    _program = ShellCommand::expand(program);
}
void Session::setInitialWorkingDirectory(const QString & dir)
{
    _initialWorkingDir = ShellCommand::expand(dir);
}
void Session::setArguments(const QStringList & arguments)
{
    _arguments = ShellCommand::expand(arguments);
}

QList<TerminalDisplay *> Session::views() const
{
    return _views;
}

void Session::addView(TerminalDisplay * widget)
{
    Q_ASSERT( !_views.contains(widget) );

    _views.append(widget);

    if ( _emulation != nullptr ) {
        // connect emulation - view signals and slots
        connect( widget , SIGNAL(keyPressedSignal(QKeyEvent *)) , _emulation ,
                 SLOT(sendKeyEvent(QKeyEvent *)) );
        connect( widget , SIGNAL(mouseSignal(int,int,int,int)) , _emulation ,
                 SLOT(sendMouseEvent(int,int,int,int)) );
        connect( widget , SIGNAL(sendStringToEmu(const char *)) , _emulation ,
                 SLOT(sendString(const char *)) );

        // allow emulation to notify view when the foreground process
        // indicates whether or not it is interested in mouse signals
        connect( _emulation , SIGNAL(programUsesMouseChanged(bool)) , widget ,
                 SLOT(setUsesMouse(bool)) );

        widget->setUsesMouse( _emulation->programUsesMouse() );

        connect( _emulation , SIGNAL(programBracketedPasteModeChanged(bool)) ,
                 widget , SLOT(setBracketedPasteMode(bool)) );

        widget->setBracketedPasteMode(_emulation->programBracketedPasteMode());

        widget->setScreenWindow(_emulation->createWindow());
    }

    //connect view signals and slots
    QObject::connect( widget ,SIGNAL(changedContentSizeSignal(int,int)),this,
                      SLOT(onViewSizeChange(int,int)));

    QObject::connect( widget ,SIGNAL(destroyed(QObject *)) , this ,
                      SLOT(viewDestroyed(QObject *)) );
//slot for close
    QObject::connect(this, SIGNAL(finished()), widget, SLOT(close()));

}

void Session::viewDestroyed(QObject * view)
{
    TerminalDisplay * display = (TerminalDisplay *)view;

    Q_ASSERT( _views.contains(display) );

    removeView(display);
}

//判断cat >>ttt<<_EOF特殊场景 //add by 2020-09-16
bool Session::specialHandle(QString &str)
{
    //防止当前消失带了上一次未发完的数据
    QStringList list = str.split("\r\u001B[K");
    str = list.at(list.size() -1);
    str.replace("\u001B[K", "");
    str.replace("\u001B[A", "");
    str.replace("\r", "");
    str.replace("\n", "");
    if(str.startsWith("> ")){
        return true;
    }else {
        return false;
    }
}

void Session::removeView(TerminalDisplay * widget)
{
    _views.removeAll(widget);

    disconnect(widget,nullptr,this,nullptr);

    if ( _emulation != nullptr ) {
        // disconnect
        //  - key presses signals from widget
        //  - mouse activity signals from widget
        //  - string sending signals from widget
        //
        //  ... and any other signals connected in addView()
        disconnect( widget, nullptr, _emulation, nullptr);

        // disconnect state change signals emitted by emulation
        disconnect( _emulation , nullptr , widget , nullptr);
    }

    // close the session automatically when the last view is removed
    if ( _views.count() == 0 ) {
        close();
    }
}

void Session::run()
{
    // Upon a KPty error, there is no description on what that error was...
    // Check to see if the given program is executable.

    /* ok iam not exactly sure where _program comes from - however it was set to /bin/bash on my system
     * Thats bad for BSD as its /usr/local/bin/bash there - its also bad for arch as its /usr/bin/bash there too!
     * So i added a check to see if /bin/bash exists - if no then we use $SHELL - if that does not exist either, we fall back to /bin/sh
     * As far as i know /bin/sh exists on every unix system.. You could also just put some ifdef __FREEBSD__ here but i think these 2 filechecks are worth
     * their computing time on any system - especially with the problem on arch linux beeing there too.
     */
    QString exec = QString::fromLocal8Bit(QFile::encodeName(_program));
    // if 'exec' is not specified, fall back to default shell.  if that
    // is not set then fall back to /bin/sh

    // here we expect full path. If there is no fullpath let's expect it's
    // a custom shell (eg. python, etc.) available in the PATH.
    if (exec.startsWith(QLatin1Char('/')) || exec.isEmpty())
    {
        const QString defaultShell{QLatin1String("/bin/sh")};

        QFile excheck(exec);
        if ( exec.isEmpty() || !excheck.exists() ) {
            exec = QString::fromLocal8Bit(qgetenv("SHELL"));
        }
        excheck.setFileName(exec);

        if ( exec.isEmpty() || !excheck.exists() ) {
            qWarning() << "Neither default shell nor $SHELL is set to a correct path. Fallback to" << defaultShell;
            exec = defaultShell;
        }
    }

    // _arguments sometimes contain ("") so isEmpty()
    // or count() does not work as expected...
    QString argsTmp(_arguments.join(QLatin1Char(' ')).trimmed());
    QStringList arguments;
    arguments << exec;
    if (argsTmp.length())
        arguments << _arguments;

    QString cwd = QDir::currentPath();
    if (!_initialWorkingDir.isEmpty()) {
        _shellProcess->setWorkingDirectory(_initialWorkingDir);
    } else {
        _shellProcess->setWorkingDirectory(cwd);
    }

    _shellProcess->setFlowControlEnabled(_flowControl);
    _shellProcess->setErase(_emulation->eraseChar());

    // this is not strictly accurate use of the COLORFGBG variable.  This does not
    // tell the terminal exactly which colors are being used, but instead approximates
    // the color scheme as "black on white" or "white on black" depending on whether
    // the background color is deemed dark or not
    QString backgroundColorHint = _hasDarkBackground ? QLatin1String("COLORFGBG=15;0") : QLatin1String("COLORFGBG=0;15");

    /* if we do all the checking if this shell exists then we use it ;)
     * Dont know about the arguments though.. maybe youll need some more checking im not sure
     * However this works on Arch and FreeBSD now.
     */
    int result = _shellProcess->start(exec,
                                      arguments,
                                      _environment << backgroundColorHint,
                                      windowId(),
                                      _addToUtmp);

    if (result < 0) {
        //qDebug() << "CRASHED! result: " << result<<arguments;
        QString infoText = QString("There was an error ctreating the child processfor this teminal. \n"
                 "Faild to execute child process \"%1\"(No such file or directory)!").arg(exec);
        sendText(infoText);
        _userTitle = QString::fromLatin1("Session crashed");
        emit titleChanged();
        return;
    }

    _shellProcess->setWriteable(false);  // We are reachable via kwrited.
    emit started();
}

void Session::runEmptyPTY()
{
    _shellProcess->setFlowControlEnabled(_flowControl);
    _shellProcess->setErase(_emulation->eraseChar());
    _shellProcess->setWriteable(false);

    // disconnet send data from emulator to internal terminal process
    disconnect( _emulation,SIGNAL(sendData(const char *,int)),
                _shellProcess, SLOT(sendData(const char *,int)) );

    _shellProcess->setEmptyPTYProperties();
    emit started();
}

void Session::setUserTitle( int what, const QString & caption )
{
    //set to true if anything is actually changed (eg. old _nameTitle != new _nameTitle )
    bool modified = false;

    // (btw: what=0 changes _userTitle and icon, what=1 only icon, what=2 only _nameTitle
    if ((what == 0) || (what == 2)) {
        _isTitleChanged = true;
        if ( _userTitle != caption ) {
            _userTitle = caption;
            modified = true;
        }
    }

    if ((what == 0) || (what == 1)) {
        _isTitleChanged = true;
        if ( _iconText != caption ) {
            _iconText = caption;
            modified = true;
        }
    }

    if (what == 11) {
        QString colorString = caption.section(QLatin1Char(';'),0,0);
        //qDebug() << __FILE__ << __LINE__ << ": setting background colour to " << colorString;
        QColor backColor = QColor(colorString);
        if (backColor.isValid()) { // change color via \033]11;Color\007
            if (backColor != _modifiedBackground) {
                _modifiedBackground = backColor;

                // bail out here until the code to connect the terminal display
                // to the changeBackgroundColor() signal has been written
                // and tested - just so we don't forget to do this.
                Q_ASSERT( 0 );

                emit changeBackgroundColorRequest(backColor);
            }
        }
    }

    if (what == 30) {
        _isTitleChanged = true;
        if ( _nameTitle != caption ) {
            setTitle(Session::NameRole,caption);
            return;
        }
    }

    if (what == 31) {
        QString cwd=caption;
        cwd=cwd.replace( QRegExp(QLatin1String("^~")), QDir::homePath() );
        emit openUrlRequest(cwd);
    }

    // change icon via \033]32;Icon\007
    if (what == 32) {
        _isTitleChanged = true;
        if ( _iconName != caption ) {
            _iconName = caption;

            modified = true;
        }
    }

    if (what == 50) {
        emit profileChangeCommandReceived(caption);
        return;
    }

    if ( modified ) {
        emit titleChanged();
    }
}

QString Session::userTitle() const
{
    return _userTitle;
}
void Session::setTabTitleFormat(TabTitleContext context , const QString & format)
{
    if ( context == LocalTabTitle ) {
        _localTabTitleFormat = format;
        ProcessInfo *process = getProcessInfo();
        process->setUserNameRequired(format.contains(QLatin1String("%u")));
    } else if ( context == RemoteTabTitle ) {
        _remoteTabTitleFormat = format;
    }
}
QString Session::tabTitleFormat(TabTitleContext context) const
{
    if ( context == LocalTabTitle ) {
        return _localTabTitleFormat;
    } else if ( context == RemoteTabTitle ) {
        return _remoteTabTitleFormat;
    }

    return QString();
}

void Session::monitorTimerDone()
{
    //FIXME: The idea here is that the notification popup will appear to tell the user than output from
    //the terminal has stopped and the popup will disappear when the user activates the session.
    //
    //This breaks with the addition of multiple views of a session.  The popup should disappear
    //when any of the views of the session becomes active


    //FIXME: Make message text for this notification and the activity notification more descriptive.
    if (_monitorSilence) {
        emit silence();
        emit stateChanged(NOTIFYSILENCE);
    } else {
        emit stateChanged(NOTIFYNORMAL);
    }

    _notifiedActivity = false;
}
//add by 2020-09-18
/*******************************************************************************
 1. @函数:    fixClearLineCmd
 2. @作者:    ut000439 王培利
 3. @日期:    2020-08-27
 4. @说明:    清行的修正处理：
              使用场景resize回来的指令，以及主动clear时候的指令
*******************************************************************************/
void Session::fixClearLineCmd(QByteArray &buffer)
{
    // \r\u001B[K 最基本的清行字符串, bash的这个字符串有问题
    QByteArray byteClearLine = "\x0d\x1b\x5b\x4b";
    // \u001B[1A  \r\u001B[K 增加清除一行的指令
    QByteArray addClearOneLine = "\x1b\x5b\x31\x41\x0d\x1b\x5b\x4b";
    // bash 有问题的\r\n\r
    QByteArray bashBugReturn = "\r\n\r";
    // \u001B[A
    QByteArray bashBugAddClearOneLine = "\x1b\x5b\x41";

    //add by ut001000 renfeixiang 2020-09-17
    //第五步消息，分开发送，变成后面的信息和resize第一步一起发送
    if(buffer.contains(byteClearLine) && m_RedrawStep == RedrawStep1_Resize_Receiving){
        QString tmpdata = buffer;
        QStringList list = tmpdata.split(byteClearLine);
        if(list.size() > 1 && list.at(0).size() > 1){
            tmpdata = tmpdata.right(tmpdata.size() - list.at(0).size());
            //使用toLatin1转换成QByteArray时，中文成乱码，改成toLocal8Bit
            buffer = tmpdata.toLocal8Bit();
            qDebug() << "change buffer" << buffer;
        }
    }

    // 正确的清除指令
    QByteArray rightClearLineCmd(byteClearLine);

    // 获取上一次一共有多少行
    /******** Modify by ut001000 renfeixiang 2020-09-09:修改 更换计算行的方法，通过存储数据的screenLines获取行数，并设置当前的cuY值 Begin***************/
//    Vt102Emulation *vt102 = static_cast<Vt102Emulation *>(_emulation);
//    int curY= vt102->_currentScreen->getCursorY() + vt102->_currentScreen->getHistLines() + 1;
//    int calLines = curY - vt102->_currentScreen->ShellStartLine + 1;
//    int cleanLineCount = calLines - 1;

    //add by 2020-09-18
    int cleanLineCount = _emulation->getCurrentScreen()->getImageHasLine()+ _emulation->getCurrentScreen()->getHistLines() - _emulation->getCurrentScreen()->getShellStartLine() - 1;
    tmpImageLine = _emulation->getCurrentScreen()->getImageHasLine();
    _emulation->getCurrentScreen()->setCursorY(tmpImageLine);
//    qDebug() << "getCursorY()" << _emulation->getCurrentScreen()->getCursorY() << "tmpImageLine" << tmpImageLine <<"_sessionId" << _sessionId;
    /******** Modify by ut001000 renfeixiang 2020-09-09 End***************/

    //防止计算的行数已经超过当前屏幕的最大行数 //add by ut001000 renfeixiang 2020-09-14
    //因为数据信息超出屏幕后，计算清行的数量，无法全部清除，超出屏幕的部分无法清除
    //只有在设置zoom时，才需要走下面的流程
    if(cleanLineCount > _emulation->getCurrentScreen()->getLines() && isZoom){
        m_RedrawStep = RedrawStep0_None;
        //zoom标志未设置为false
        isZoom = false;
        return;
    }else {
        //正常流程,zoom标志未设置为false
        isZoom = false;
    }

    // 把原来指令中的相关清行指令还原成最原始的清行指令
    // 组装应该有的清行信息
    for (int i = 1; i <= cleanLineCount; i++)
    {
        rightClearLineCmd.append(addClearOneLine);
    }

    if(m_RedrawStep == RedrawStep1_Resize_Receiving)
    {
        if(buffer.endsWith(" \r"))
        {
            buffer = buffer.left(buffer.length() - 2);
        }
    }

    // resize最后的信息不知为何为加几个\b,全干掉
    for (int i = 0; i <= _views[0]->lines(); i++)
    {
        // 不知为何bash指令，有时会多很多清行信息，全干掉
        buffer.replace(byteClearLine + bashBugAddClearOneLine, byteClearLine);
        if (buffer.endsWith("\b"))
        {
            //qDebug() << "clear \\b: "<<buffer;
            buffer = buffer.left(buffer.size() - 1);
        }
    }
    // 将前面处理过的原始清行指令替换成该有的清行指令
    buffer.replace(byteClearLine, rightClearLineCmd);

    // 把有问题的\r\n\r 删除
    buffer.replace(bashBugReturn, "");

    /******** Add by ut001000 renfeixiang 2020-09-09:增加，在有多行输入信息时，并且光标往上面行移动时的场景，
     * 收到的bash信息解析后，信息前面多了\n，需要删除 Begin***************/
    while(buffer.startsWith("\n")){
        buffer = buffer.right(buffer.size() - 1);
    }
    /******** Add by ut001000 renfeixiang 2020-09-09 End***************/

//    qDebug()<<"fix buffer:"<<buffer;
}

void Session::activityStateSet(int state)
{
    if (state==NOTIFYBELL) {
        emit bellRequest(QString("Bell in session '%1'").arg(_nameTitle));
    } else if (state==NOTIFYACTIVITY) {
        if (_monitorSilence) {
            _monitorTimer->start(_silenceSeconds*1000);
        }

        if ( _monitorActivity ) {
            //FIXME:  See comments in Session::monitorTimerDone()
            if (!_notifiedActivity) {
                _notifiedActivity=true;
                emit activity();
            }
        }
    }

    if ( state==NOTIFYACTIVITY && !_monitorActivity ) {
        state = NOTIFYNORMAL;
    }
    if ( state==NOTIFYSILENCE && !_monitorSilence ) {
        state = NOTIFYNORMAL;
    }

    emit stateChanged(state);
}

void Session::onViewSizeChange(int height, int width)
{
    updateTerminalSize(height, width);
}
void Session::onEmulationSizeChange(QSize size)
{
    setSize(size);
}

void Session::updateTerminalSize(int height, int width)
{
    QListIterator<TerminalDisplay *> viewIter(_views);

    int minLines = -1;
    int minColumns = -1;

    // minimum number of lines and columns that views require for
    // their size to be taken into consideration ( to avoid problems
    // with new view widgets which haven't yet been set to their correct size )
    /***mod by ut001121 zhangmeng 20200615 窗口初始化完成之后修改终端显示内容为最小一行 修复BUG32779, BUG32778***/
    const int VIEW_LINES_THRESHOLD = (height==1 || width==1) ? 2 : 1;
    const int VIEW_COLUMNS_THRESHOLD = 2;

    //select largest number of lines and columns that will fit in all visible views
    while ( viewIter.hasNext() ) {
        TerminalDisplay * view = viewIter.next();
        if ( view->isHidden() == false &&
                view->lines() >= VIEW_LINES_THRESHOLD &&
                view->columns() >= VIEW_COLUMNS_THRESHOLD ) {
            minLines = (minLines == -1) ? view->lines() : qMin( minLines , view->lines() );
            minColumns = (minColumns == -1) ? view->columns() : qMin( minColumns , view->columns() );
        }
    }

    // backend emulation must have a _terminal of at least 1 column x 1 line in size
    if (minLines > 0 && minColumns > 0)
    {
        //add by 2020-09-18 //为什么注释掉这个setImageSize函数
        //_emulation->setImageSize(minLines, minColumns);
        //add by ut001000 renfeixiang 2020-09-16
        //特殊场景，不需要TerminalDisplay上行列进行更新
        if(!IsSpecialscene){
            _shellProcess->setWindowSize(minLines, minColumns);
        }
    }
}

void Session::refresh()
{
    // attempt to get the shell process to redraw the display
    //
    // this requires the program running in the shell
    // to cooperate by sending an update in response to
    // a window size change
    //
    // the window size is changed twice, first made slightly larger and then
    // resized back to its normal size so that there is actually a change
    // in the window size (some shells do nothing if the
    // new and old sizes are the same)
    //
    // if there is a more 'correct' way to do this, please
    // send an email with method or patches to konsole-devel@kde.org

    const QSize existingSize = _shellProcess->windowSize();
    _shellProcess->setWindowSize(existingSize.height(),existingSize.width()+1);
    _shellProcess->setWindowSize(existingSize.height(),existingSize.width());
}

bool Session::sendSignal(int signal)
{
    int result = ::kill(_shellProcess->pid(),signal);

     if ( result == 0 )
     {
         _shellProcess->waitForFinished();
         return true;
     }
     else
         return false;
}

void Session::close()
{
    _autoClose = true;
    _wantedClose = true;
    if (!_shellProcess->isRunning() || !sendSignal(SIGHUP)) {
        // Forced close.
        QTimer::singleShot(1, this, SIGNAL(finished()));
    }
}

void Session::sendText(const QString & text) const
{
    _emulation->sendText(text);
}

void Session::sendKeyEvent(QKeyEvent* e) const
{
    _emulation->sendKeyEvent(e);
}

Session::~Session()
{
    _wantedClose = true;
    if(nullptr != _foregroundProcessInfo){
        delete _foregroundProcessInfo;
    }
    if(nullptr != _sessionProcessInfo){
        delete _sessionProcessInfo;
    }
    if(nullptr != _emulation){
        delete _emulation;
    }
    if(nullptr != _shellProcess){
        delete _shellProcess;
    }
    //  delete _zmodemProc;
}
//add by 2020-09-18
void Session::entryRedrawStep(Session::RedrawStep step)
{
//    qDebug()<<"entryRedrawStep"<<QMetaEnum::fromType<RedrawStep>().key(step);
    m_RedrawStep = step;
}
//add by 2020-09-18
bool Session::preRedraw(QByteArray & data)
{
    if(!_shellProcess->getResizeMode())
    {
        return true;
    }
    QByteArray byteClear("\x0d\x1b\x5b\x4b\x1b\x5b\x41\u0007");
    //用于判断cat >>ttt<<_EOF特殊场景
    QString tmpstrdata = "";

    switch (m_RedrawStep) {
    case RedrawStep1_Resize_Receiving://将非resize时保存的信息清除
//        qDebug()<<m_RedrawStep <<" OK!" << "_sessionId" << _sessionId;
        _emulation->setResizeSaveType(Emulation::SaveAll);
        _emulation->clearResizeAllString();
        /******** Add by ut001000 renfeixiang 2020-09-09:增加,用于屏蔽分屏幕场景，第一条消息，被分开发送，分开的消息发送到resize流程中 Begin***************/
        //增加特殊场景开头是>，不进行resize操作，判断cat >>ttt<<_EOF特殊场景
        tmpstrdata = QString::fromLocal8Bit(data);
        if(!data.contains("\x0d\x1b\x5b\x4b") || specialHandle(tmpstrdata)){
            m_RedrawStep = RedrawStep0_None;
            break;
        }
        /******** Add by ut001000 renfeixiang 2020-09-09 End***************/
        fixClearLineCmd(data);

        break;
    case RedrawStep1_Ctrl_u_Receiving:
        //增加 data.contains("\b") 场景的判断，有的场景第三步只发送了一个\b回来 //add by ut001000 renfeixiang 2020-09-14 //使用toLatin1转换成QByteArray时，中文成乱码，改成toLocal8Bit
        if(((data == "\x07") || data.contains("\u001B[C") || data.contains("\u001B[K") || data.contains("\b")) && !data.endsWith(_emulation->getResizeAllString().toLocal8Bit()))
        {
            //qDebug()<<"Step1_Ctrl_u = true, ignore dataReceived" <<data<<"resizeAllString"<<_emulation->getResizeAllString();
            entryRedrawStep(RedrawStep2_Clear_Receiving);
            _emulation->setResizeSaveType(Emulation::SaveNone);
            data = byteClear;
            fixClearLineCmd(data);
            return true;
        }
        qDebug()<<"info is not crrect !"<< data;
        return false;
    case RedrawStep3_Return_Receiving:
        deleteReturnChar(data);
        if(!data.contains("\x07"))
        {
            return false;
        }
        break;
    default:
        break;
    }

    return  true;
}
//add by 2020-09-18
void Session::tailRedraw()
{
    // 每次重绘的入口
    onRedrawData(m_RedrawStep);
}
//add by 2020-09-18
void Session::deleteReturnChar(QByteArray &data)
{
    bool deleteOK = false;
    QByteArrayList maybePreList;
    maybePreList<<"\r\n"<<"\u0007\r\n"<<"\u001B[A\r\n";

    /******** Add by ut001000 renfeixiang 2020-09-09:增加 用于计算出第三步正确的接收信息，格式\r\nXX\u0007XX Begin***************/
    QString strbyte = data;
    strbyte = strbyte.replace("\r\n\r","");
    lastRecvReturnData = lastRecvReturnData+strbyte;
//    qDebug() << "lastRecvReturnDatalastRecvReturnData" << lastRecvReturnData;
    if(lastRecvReturnData.contains("\r\n") && lastRecvReturnData.contains("\x07")){
        //增加第三布保存信息的校验，屏蔽\x07后面没有信息也走正常流程的问题
        QStringList list = lastRecvReturnData.split("\x07");
        //有可能收到的信息结尾是\r\n\r，但是bash分开发送变成\r\n 和 \r //add by ut001000 renfeixiang 2020-09-17
        QStringList listEnter = lastRecvReturnData.split("\r\n");
        if(list.at(list.size() - 1).size() > 0 && listEnter.at(listEnter.size() - 1).size() > 0){
            strbyte = lastRecvReturnData;
            lastRecvReturnData = "";
        }else {
            data.replace("\x07","");
            return;
        }
    }else {
        data.replace("\x07","");
        return;
    }

    //如果bash分开发送变成\r\n 和 \r，拼接后会存在\r\n\r //add by ut001000 renfeixiang 2020-09-17
    strbyte = strbyte.replace("\r\n\r","");

    if(strbyte.contains("\r\n")){
        QStringList list = strbyte.split("\r\n");
//        qDebug() << "list.size" << list.size();
        if(list.size() > 1){
            strbyte = "\r\n";
            strbyte += list.at(list.size() - 1);
            //使用toLatin1转换成QByteArray时，中文成乱码，改成toLocal8Bit
            data = strbyte.toLocal8Bit();
//            qDebug() << "strbytestrbyte" << strbyte;
        }
    }
    /******** Add by ut001000 renfeixiang 2020-09-09 End***************/

    for(QByteArray pre: maybePreList)
    {
        if(data.startsWith(pre) )
        {
//            qDebug()<<"deleteReturnChar"<<data;
            data = data.right(data.size() - pre.length());
            deleteOK = true;
            break;
        }
    }
    if(deleteOK)
    {
//        qDebug()<<m_RedrawStep <<" OK!";
        _emulation->setResizeSaveType(Emulation::SavePrompt);
        _emulation->setHasMorespace(false);
        _emulation->clearStartPrompt();
        _emulation->clearSwapByte();
    }
    if(data.endsWith(" \r"))
    {
        qDebug()<<data<<"has \\r";
        _emulation->setHasMorespace(true);
    }
}
//add by 2020-09-18
void Session::onRedrawData(RedrawStep step)
{
    m_RedrawStep = step;
    entryRedrawStep(m_RedrawStep);
    //add by ut001000 renfeixiang 2020-09-14
    //增加\u001B[F让光标回到消息的最后，防止出现光标在信息最开始，输入回车后，把输入信息执行了
    QByteArray byteCtrlU("\u001B[F\u0015");//byteCtrlU("\u001B[F\u0015");
    QByteArray byteReturn("\r");
    QByteArray byteSwapText;
    //add by ut001000 renfeixiang 2020-09-16
    //用于计算当前的shell信息是否满足特殊场景
    int tmpnumber = 0;
    switch (step) {
    case Session::RedrawStep0_None:
         //qDebug()<<" resize continue ?????????????";
        if(_shellProcess->isNeeadResize())
        {
            qDebug()<<" resize continue..." << "_sessionId" << _sessionId;
            _shellProcess->startResize();
        }
        return;

    case Session::RedrawStep1_Resize_Receiving:
        if(_emulation->getResizeAllString().isEmpty())
        {
            return;
        }
        /******** Modify by ut001000 renfeixiang 2020-09-09:修改,在清除行前，设置下光标的行数，主要是为了光标移动的场景 Begin***************/
        _emulation->getCurrentScreen()->setCursorY(tmpImageLine);
         /******** Modify by ut001000 renfeixiang 2020-09-09: End***************/
        _emulation->sendString(byteCtrlU.data(), byteCtrlU.count(), true);
        break;

    case Session::RedrawStep1_Ctrl_u_Receiving:
        return;
    case Session::RedrawStep2_Clear_Receiving:
        _emulation->sendString(byteReturn.data(), byteReturn.count(), true);
        break;
    case Session::RedrawStep3_Return_Receiving:
        //发现resizeAllString信息后面也包含了“ \r” //add by ut001000 renfeixiang 2020-09-14
        if(_emulation->getResizeAllString().endsWith("  \r"))
        {
            QString tmpAllString = _emulation->getResizeAllString();
            tmpAllString = tmpAllString.left(tmpAllString.size() - 2);
            _emulation->setResizeAllString(tmpAllString);
        }
//        qDebug()<<"startPrompt"<<_emulation->getStartPrompt();
//        qDebug()<<"resizeAllString"<<_emulation->getResizeAllString();
        // 有的时候会发来的消息只有\0007信息
        if(_emulation->getStartPrompt().isEmpty())
        {
            return;
        }
        if(_emulation->getHasMorespace())
        {
            QString tmpStartPrompt = _emulation->getStartPrompt();
            tmpStartPrompt = tmpStartPrompt.left(tmpStartPrompt.size() - 1);
            _emulation->setStartPrompt(tmpStartPrompt);
        }

        /******** Add by ut001000 renfeixiang 2020-09-16:增加 Begin***************/
        //对特殊场景，进行一次resize重绘处理，将当前的colomn减一，IsSpecialscene标记只进行一次重绘
        //qDebug() << "_sessionId" << _sessionId << "IsSpecialscene" << IsSpecialscene;
        if(!IsSpecialscene){
            tmpnumber = _emulation->getStartPrompt().size() - _emulation->getCurrentScreen()->getColumns()*2;
            //特殊场景，有可能是光标在第三行行首，或者第二个位置
            if(tmpnumber >= 0 && tmpnumber < 2)
            {
//                qDebug() << "tmpnumber" << tmpnumber << "_emulation->startPrompt.size()" << _emulation->getStartPrompt().size() << "_emulation->_currentScreen->getColumns()" << _emulation->getCurrentScreen()->getColumns();
                qDebug() << "enter special scene";
                if(!_shellProcess->isNeeadResize()){
                    _shellProcess->setSwapWindowColumns(_shellProcess->getSwapWindowColumns() - 1);
//                    qDebug() << _shellProcess->swap_windowColumns << _shellProcess->redraw_windowColumns;
                    IsSpecialscene = true;
                }
            }
        }else {
            IsSpecialscene = false;
        }
        /******** Add by ut001000 renfeixiang 2020-09-16:增加 End***************/

        /******** Add by ut001000 renfeixiang 2020-09-09:增加 将resizeAllString的信息格式化，开始的信息必须是shell信息 Begin***************/
        //这种场景估计不会在出现了，因为resize第一步接收的消息已经在了格式校验，这个问题当时出现是export “%%%%%%”场景
        if(!_emulation->getResizeAllString().startsWith(_emulation->getStartPrompt())){
            if(_emulation->getResizeAllString().contains(_emulation->getStartPrompt())){
                QStringList list = _emulation->getResizeAllString().split(_emulation->getStartPrompt());
                if(list.size() > 1){
                    _emulation->setResizeAllString(_emulation->getStartPrompt()+list.at(0)+list.at(1));
                    qDebug() << "resizeAllString format is error, show change resizeAllString:" << _emulation->getResizeAllString();
                }
            }
        }
        /******** Add by ut001000 renfeixiang 2020-09-09 End***************/
        if(_emulation->getResizeAllString().startsWith(_emulation->getStartPrompt()) )
        {
            _emulation->setResizeSaveType(Emulation::SaveNone);
            _emulation->setSwapByte(_emulation->getResizeAllString().right(_emulation->getResizeAllString().size()
                                                                           - _emulation->getStartPrompt().size()));
            _emulation->clearResizeAllString();
            _emulation->clearStartPrompt();
            byteSwapText.append(_emulation->getSwapByte());
            if(byteSwapText.count() != 0)
            {
                entryRedrawStep(RedrawStep4_SwapText);
                _emulation->sendString(byteSwapText.data(),byteSwapText.count(), true);
            }
            else {
                qDebug()<<"none of swapByte will be send";
                onRedrawData(RedrawStep4_SwapText);
                return;
            }
        }else {
            qDebug()<<"resizeAllString format is error";
            //resizeAllString信息格式错误时，继续走正常流程不能卡住
            onRedrawData(RedrawStep4_SwapText);
            return;
        }
//        qDebug()<<"swapByte"<<_emulation->getSwapByte();
        break;

    case Session::RedrawStep4_SwapText:
        _emulation->sendString("",0, true);
        return;
    //正常情况走，发送空时不走
    case Session::RedrawStep5_UserKey:
        _emulation->sendString("",0, true);
        return;
    default:
        break;
    }
    // 如果流程处理完了，回来的时候就变成了RedrawStep0_None
    if(m_RedrawStep != RedrawStep0_None)
    {
        m_RedrawStep = RedrawStep(1+ m_RedrawStep);
    }
}

void Session::setProfileKey(const QString & key)
{
    _profileKey = key;
    emit profileChanged(key);
}
QString Session::profileKey() const
{
    return _profileKey;
}

void Session::done(int exitStatus)
{
    qDebug()<<"done exitStatus:"<<exitStatus<< _shellProcess->exitStatus();
    if (_autoClose || _wantedClose) {
        emit finished();
        return;
    }
    if(exitStatus != 0)
    {
        QString message;
        QString infoText;
        if (exitStatus == -1){
            infoText.sprintf("There was an error ctreating the child processfor this teminal. \n"
                     "Faild to execute child process \"%s\"(No such file or directory)!", _program.toUtf8().data());
            message = "Session crashed.";
        }
        else {
            infoText.sprintf("The child process exit normally with status %d.", exitStatus);
            message.sprintf("Session '%s' exited with status %d.",
                      _nameTitle.toUtf8().data(), exitStatus);
        }
        _userTitle = message;
        //sendText(infoText);
        emit titleChanged();
    }
}

Emulation * Session::emulation() const
{
    return _emulation;
}

QString Session::keyBindings() const
{
    return _emulation->keyBindings();
}

void Session::setBackspaceMode(char *key, int length)
{
    _emulation->setBackspaceMode(key, length);
}

void Session::setDeleteMode(char *key, int length)
{
    _emulation->setDeleteMode(key, length);
}

QStringList Session::environment() const
{
    return _environment;
}

void Session::setEnvironment(const QStringList & environment)
{
    _environment = environment;
}

int Session::sessionId() const
{
    return _sessionId;
}

void Session::setKeyBindings(const QString & id)
{
    _emulation->setKeyBindings(id);
}

void Session::setTitle(TitleRole role , const QString & newTitle)
{
    if ( title(role) != newTitle ) {
        if ( role == NameRole ) {
            _nameTitle = newTitle;
        } else if ( role == DisplayedTitleRole ) {
            _displayTitle = newTitle;
            // without these, that title will be overridden by the expansion of
            // title format shortly after, which will confuses users.
            _localTabTitleFormat = newTitle;
            _remoteTabTitleFormat = newTitle;

//            qDebug() << "curr running process:" << newTitle << endl;
        }

        emit titleChanged();
    }
}

QString Session::title(TitleRole role) const
{
    if ( role == NameRole ) {
        return _nameTitle;
    } else if ( role == DisplayedTitleRole ) {
        return _displayTitle;
    } else {
        return QString();
    }
}

void Session::setIconName(const QString & iconName)
{
    if ( iconName != _iconName ) {
        _iconName = iconName;
        emit titleChanged();
    }
}

void Session::setIconText(const QString & iconText)
{
    _iconText = iconText;
    //kDebug(1211)<<"Session setIconText " <<  _iconText;
}

QString Session::iconName() const
{
    return _iconName;
}

QString Session::iconText() const
{
    return _iconText;
}

bool Session::isTitleChanged() const
{
    return _isTitleChanged;
}

void Session::setHistoryType(const HistoryType & hType)
{
    _emulation->setHistory(hType);
}

const HistoryType & Session::historyType() const
{
    return _emulation->history();
}

void Session::clearHistory()
{
    _emulation->clearHistory();
}

QStringList Session::arguments() const
{
    return _arguments;
}

QString Session::program() const
{
    return _program;
}

// unused currently
bool Session::isMonitorActivity() const
{
    return _monitorActivity;
}
// unused currently
bool Session::isMonitorSilence()  const
{
    return _monitorSilence;
}

void Session::setMonitorActivity(bool _monitor)
{
    _monitorActivity=_monitor;
    _notifiedActivity=false;

    activityStateSet(NOTIFYNORMAL);
}

void Session::setMonitorSilence(bool _monitor)
{
    if (_monitorSilence==_monitor) {
        return;
    }

    _monitorSilence=_monitor;
    if (_monitorSilence) {
        _monitorTimer->start(_silenceSeconds*1000);
    } else {
        _monitorTimer->stop();
    }

    activityStateSet(NOTIFYNORMAL);
}

void Session::setMonitorSilenceSeconds(int seconds)
{
    _silenceSeconds=seconds;
    if (_monitorSilence) {
        _monitorTimer->start(_silenceSeconds*1000);
    }
}

void Session::setAddToUtmp(bool set)
{
    _addToUtmp = set;
}

void Session::setFlowControlEnabled(bool enabled)
{
    if (_flowControl == enabled) {
        return;
    }

    _flowControl = enabled;

    if (_shellProcess) {
        _shellProcess->setFlowControlEnabled(_flowControl);
    }

    emit flowControlEnabledChanged(enabled);
}
bool Session::flowControlEnabled() const
{
    return _flowControl;
}
//void Session::fireZModemDetected()
//{
//  if (!_zmodemBusy)
//  {
//    QTimer::singleShot(10, this, SIGNAL(zmodemDetected()));
//    _zmodemBusy = true;
//  }
//}

//void Session::cancelZModem()
//{
//  _shellProcess->sendData("\030\030\030\030", 4); // Abort
//  _zmodemBusy = false;
//}

//void Session::startZModem(const QString &zmodem, const QString &dir, const QStringList &list)
//{
//  _zmodemBusy = true;
//  _zmodemProc = new KProcess();
//  _zmodemProc->setOutputChannelMode( KProcess::SeparateChannels );
//
//  *_zmodemProc << zmodem << "-v" << list;
//
//  if (!dir.isEmpty())
//     _zmodemProc->setWorkingDirectory(dir);
//
//  _zmodemProc->start();
//
//  connect(_zmodemProc,SIGNAL (readyReadStandardOutput()),
//          this, SLOT(zmodemReadAndSendBlock()));
//  connect(_zmodemProc,SIGNAL (readyReadStandardError()),
//          this, SLOT(zmodemReadStatus()));
//  connect(_zmodemProc,SIGNAL (finished(int,QProcess::ExitStatus)),
//          this, SLOT(zmodemFinished()));
//
//  disconnect( _shellProcess,SIGNAL(block_in(const char*,int)), this, SLOT(onReceiveBlock(const char*,int)) );
//  connect( _shellProcess,SIGNAL(block_in(const char*,int)), this, SLOT(zmodemRcvBlock(const char*,int)) );
//
//  _zmodemProgress = new ZModemDialog(QApplication::activeWindow(), false,
//                                    i18n("ZModem Progress"));
//
//  connect(_zmodemProgress, SIGNAL(user1Clicked()),
//          this, SLOT(zmodemDone()));
//
//  _zmodemProgress->show();
//}

/*void Session::zmodemReadAndSendBlock()
{
  _zmodemProc->setReadChannel( QProcess::StandardOutput );
  QByteArray data = _zmodemProc->readAll();

  if ( data.count() == 0 )
      return;

  _shellProcess->sendData(data.constData(),data.count());
}
*/
/*
void Session::zmodemReadStatus()
{
  _zmodemProc->setReadChannel( QProcess::StandardError );
  QByteArray msg = _zmodemProc->readAll();
  while(!msg.isEmpty())
  {
     int i = msg.indexOf('\015');
     int j = msg.indexOf('\012');
     QByteArray txt;
     if ((i != -1) && ((j == -1) || (i < j)))
     {
       msg = msg.mid(i+1);
     }
     else if (j != -1)
     {
       txt = msg.left(j);
       msg = msg.mid(j+1);
     }
     else
     {
       txt = msg;
       msg.truncate(0);
     }
     if (!txt.isEmpty())
       _zmodemProgress->addProgressText(QString::fromLocal8Bit(txt));
  }
}
*/
/*
void Session::zmodemRcvBlock(const char *data, int len)
{
  QByteArray ba( data, len );

  _zmodemProc->write( ba );
}
*/
/*
void Session::zmodemFinished()
{
  if (_zmodemProc)
  {
    delete _zmodemProc;
    _zmodemProc = 0;
    _zmodemBusy = false;

    disconnect( _shellProcess,SIGNAL(block_in(const char*,int)), this ,SLOT(zmodemRcvBlock(const char*,int)) );
    connect( _shellProcess,SIGNAL(block_in(const char*,int)), this, SLOT(onReceiveBlock(const char*,int)) );

    _shellProcess->sendData("\030\030\030\030", 4); // Abort
    _shellProcess->sendData("\001\013\n", 3); // Try to get prompt back
    _zmodemProgress->transferDone();
  }
}
*/
void Session::onReceiveBlock(const char *buf, int len)
{
    //add by 2020-09-18 begin
//    qDebug() <<m_RedrawStep <<_emulation->getResizeAllString()<< "new onReceiveBlock" << QString::fromLocal8Bit(buf, len)
//            << "_sessionId" << _sessionId;
    QByteArray byteBuf(buf, len);

    // 重绘的预处理
    if(!preRedraw(byteBuf))
    {
//        qDebug()<<m_RedrawStep<<" data is delete"<<byteBuf;
        return;
    }

    if(byteBuf.count() == 0)
    {
        return;
    }
    // 解码的入口
    _emulation->receiveData(byteBuf.data(), byteBuf.length());

    // 重绘的尾处理，要解码信息
    tailRedraw();

    emit receivedData(QString::fromLatin1(byteBuf.data(), byteBuf.length()));

    //add by ut001000 renfeixiang 2020-09-17
    //增加特殊场景处理，当分屏幕时，分屏幕收到的信息分开发送，第二条信息进入到resize流程中
    if(m_RedrawStep == RedrawStep0_None){
        //判断条件错误，有的正常命令也会最后带\r\n\r，条件改为空格+\r\n\r
        if(byteBuf.endsWith(" \r\n\r") || byteBuf.endsWith("\r\n\r ")){
            qDebug() << "Specialscene resize";
            //_shellProcess->swap_windowColumns -= 1;
            _shellProcess->setSwapWindowColumns(_shellProcess->getSwapWindowColumns() - 1);
            _shellProcess->startResize();
        }
    }
    //add by 2020-09-18 end
}

QSize Session::size()
{
    return _emulation->imageSize();
}

void Session::setSize(const QSize & size)
{
    if ((size.width() <= 1) || (size.height() <= 1)) {
        return;
    }

    emit resizeRequest(size);
}

QString Session::getDynamicProcessName()
{
    bool ok = false;
    QString processName = getProcessInfo()->name(&ok);
    if (ok) {
        return processName;
    }

    return QString::fromLocal8Bit(qgetenv("SHELL"));
}

int Session::getDynamicProcessId()
{
    bool ok = false;
    int pid = getProcessInfo()->pid(&ok);
    if (ok) {
        if (this->isForegroundProcessActive()) {
            return pid;
        }
    }

    return 0;
}

void Session::updateWorkingDirectory()
{
    updateSessionProcessInfo();

    const QString currentDir = _sessionProcessInfo->validCurrentDir();
    if (currentDir != _currentWorkingDir) {
        _currentWorkingDir = currentDir;
        emit currentDirectoryChanged(_currentWorkingDir);
    }
}

int Session::foregroundProcessId()
{
    int pid;

    bool ok = false;
    pid = getProcessInfo()->pid(&ok);
    if (!ok) {
        pid = -1;
    }

    return pid;
}

bool Session::isForegroundProcessActive()
{
    // foreground process info is always updated after this
    return (_shellProcess->pid() != _shellProcess->foregroundProcessGroup());
}

QString Session::foregroundProcessName()
{
    QString name;

    if (updateForegroundProcessInfo()) {
        bool ok = false;
        name = _foregroundProcessInfo->name(&ok);
        if (!ok) {
            name.clear();
        }
    }

    return name;
}

int Session::processId() const
{
    return _shellProcess->pid();
}

ProcessInfo *Session::getProcessInfo()
{
    ProcessInfo *process = nullptr;

    if (isForegroundProcessActive() && updateForegroundProcessInfo()) {
        process = _foregroundProcessInfo;
    } else {
        updateSessionProcessInfo();
        process = _sessionProcessInfo;
    }

    return process;
}

void Session::updateSessionProcessInfo()
{
    Q_ASSERT(_shellProcess);

    bool ok;
    // The checking for pid changing looks stupid, but it is needed
    // at the moment to workaround the problem that processId() might
    // return 0
    if ((_sessionProcessInfo == nullptr) ||
            (processId() != 0 && processId() != _sessionProcessInfo->pid(&ok))) {
        delete _sessionProcessInfo;
        _sessionProcessInfo = ProcessInfo::newInstance(processId(),
                                                       tabTitleFormat(Session::LocalTabTitle));
        _sessionProcessInfo->setUserHomeDir();
    }
    _sessionProcessInfo->update();
}

bool Session::updateForegroundProcessInfo()
{
    Q_ASSERT(_shellProcess);

    const int foregroundPid = _shellProcess->foregroundProcessGroup();
    if (foregroundPid != _foregroundPid) {
        if(nullptr != _foregroundProcessInfo){
            delete _foregroundProcessInfo;
        }

        _foregroundProcessInfo = ProcessInfo::newInstance(foregroundPid,
                                                          tabTitleFormat(Session::LocalTabTitle));
        _foregroundPid = foregroundPid;
    }

    if (_foregroundProcessInfo != nullptr) {
        _foregroundProcessInfo->update();
        return _foregroundProcessInfo->isValid();
    } else {
        return false;
    }
}

int Session::getPtySlaveFd() const
{
    return ptySlaveFd;
}

/*******************************************************************************
 1. @函数:    getEraseChar
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-06-02
 4. @说明:    获取tty的erase的字符
*******************************************************************************/
char Session::getEraseChar()
{
    return _shellProcess->erase();
}

SessionGroup::SessionGroup()
        : _masterMode(0)
{
}
SessionGroup::~SessionGroup()
{
    // disconnect all
    connectAll(false);
}
int SessionGroup::masterMode() const
{
    return _masterMode;
}
QList<Session *> SessionGroup::sessions() const
{
    return _sessions.keys();
}
bool SessionGroup::masterStatus(Session * session) const
{
    return _sessions[session];
}

void SessionGroup::addSession(Session * session)
{
    _sessions.insert(session,false);

    QListIterator<Session *> masterIter(masters());

    while ( masterIter.hasNext() ) {
        connectPair(masterIter.next(),session);
    }
}
void SessionGroup::removeSession(Session * session)
{
    setMasterStatus(session,false);

    QListIterator<Session *> masterIter(masters());

    while ( masterIter.hasNext() ) {
        disconnectPair(masterIter.next(),session);
    }

    _sessions.remove(session);
}
void SessionGroup::setMasterMode(int mode)
{
    _masterMode = mode;

    connectAll(false);
    connectAll(true);
}
QList<Session *> SessionGroup::masters() const
{
    return _sessions.keys(true);
}
void SessionGroup::connectAll(bool connect)
{
    QListIterator<Session *> masterIter(masters());

    while ( masterIter.hasNext() ) {
        Session * master = masterIter.next();

        QListIterator<Session *> otherIter(_sessions.keys());
        while ( otherIter.hasNext() ) {
            Session * other = otherIter.next();

            if ( other != master ) {
                if ( connect ) {
                    connectPair(master,other);
                } else {
                    disconnectPair(master,other);
                }
            }
        }
    }
}
void SessionGroup::setMasterStatus(Session * session, bool master)
{
    bool wasMaster = _sessions[session];
    _sessions[session] = master;

    if ((!wasMaster && !master)
            || (wasMaster && master)) {
        return;
    }

    QListIterator<Session *> iter(_sessions.keys());
    while (iter.hasNext()) {
        Session * other = iter.next();

        if (other != session) {
            if (master) {
                connectPair(session, other);
            } else {
                disconnectPair(session, other);
            }
        }
    }
}

void SessionGroup::connectPair(Session * master , Session * other)
{
//    qDebug() << k_funcinfo;

    if ( _masterMode & CopyInputToAll ) {
        qDebug() << "Connection session " << master->nameTitle() << "to" << other->nameTitle();

        connect( master->emulation() , SIGNAL(sendData(const char *,int)) , other->emulation() ,
                 SLOT(sendString(const char *,int)) );
    }
}
void SessionGroup::disconnectPair(Session * master , Session * other)
{
//    qDebug() << k_funcinfo;

    if ( _masterMode & CopyInputToAll ) {
        qDebug() << "Disconnecting session " << master->nameTitle() << "from" << other->nameTitle();

        disconnect( master->emulation() , SIGNAL(sendData(const char *,int)) , other->emulation() ,
                    SLOT(sendString(const char *,int)) );
    }
}

//#include "moc_Session.cpp"
