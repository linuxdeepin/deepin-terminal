/*
    This file is part of Konsole

    Copyright (C) 2006-2007 by Robert Knight <robertknight@gmail.com>
    Copyright (C) 1997,1998 by Lars Doelle <lars.doelle@on-line.de>

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

    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>, Copyright (C)2008
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

#include "Pty.h"
#include "ProcessInfo.h"
//#include "kptyprocess.h"
#include "TerminalDisplay.h"
#include "ShellCommand.h"
#include "Vt102Emulation.h"

using namespace Konsole;

int Session::lastSessionId = 0;

Session::Session(QObject* parent) :
    QObject(parent),
        _shellProcess(nullptr)
        , _emulation(nullptr)
        , _monitorActivity(false)
        , _monitorSilence(false)
        , _notifiedActivity(false)
        , _autoClose(true)
        , _wantedClose(false)
        , _silenceSeconds(10)
        , _isTitleChanged(false)
        , _addToUtmp(false)  // disabled by default because of a bug encountered on certain systems
        // which caused Konsole to hang when closing a tab and then opening a new
        // one.  A 'QProcess destroyed while still running' warning was being
        // printed to the terminal.  Likely a problem in KPty::logout()
        // or KPty::login() which uses a QProcess to start /usr/bin/utempter
        , _flowControl(true)
        , _fullScripting(false)
        , _sessionId(0)
    	, _sessionProcessInfo(nullptr)
    	, _foregroundProcessInfo(nullptr)
//   , _zmodemBusy(false)
//   , _zmodemProc(0)
//   , _zmodemProgress(0)
        , _hasDarkBackground(false)
        , _isPrimaryScreen(true)
{
    //prepare DBus communication
//    new SessionAdaptor(this);
    _sessionId = ++lastSessionId;
//    QDBusConnection::sessionBus().registerObject(QLatin1String("/Sessions/")+QString::number(_sessionId), this);

    //create teletype for I/O with shell process
    _shellProcess = new Pty();
    _shellProcess->setSessionId(_sessionId);
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
    connect(_emulation, &Konsole::Emulation::primaryScreenInUse,
            this, &Session::onPrimaryScreenInUse);
    connect(_emulation, SIGNAL(imageResizeRequest(QSize)),
            this, SLOT(onEmulationSizeChange(QSize)));
    connect(_emulation, SIGNAL(imageSizeChanged(int, int)),
            this, SLOT(onViewSizeChange(int, int)));
    connect(_emulation, &Vt102Emulation::cursorChanged,
            this, &Session::cursorChanged);

    //connect teletype to emulation backend
    _shellProcess->setUtf8Mode(_emulation->utf8());

    connect( _shellProcess,SIGNAL(receivedData(const char *,int,bool)),this,
             SLOT(onReceiveBlock(const char *,int,bool)) );
    connect( _emulation,SIGNAL(sendData(const char *,int,const QTextCodec *)),_shellProcess,
             SLOT(sendData(const char *,int,const QTextCodec *)) );
    connect( _emulation,SIGNAL(lockPtyRequest(bool)),_shellProcess,SLOT(lockPty(bool)) );
    connect( _emulation,SIGNAL(useUtf8Request(bool)),_shellProcess,SLOT(setUtf8Mode(bool)) );

    connect( _shellProcess,SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(done(int)) );
    // not in kprocess anymore connect( _shellProcess,SIGNAL(done(int)), this, SLOT(done(int)) );

    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/
    // 用于卸载终端弹出框提示
    connect( _shellProcess,SIGNAL(ptyUninstallTerminal(QString)), this, SIGNAL(sessionUninstallTerminal(QString)) );
    /******** Modify by nt001000 renfeixiang 2020-05-27:修改 增加参数区别remove和purge卸载命令 Begin***************/

    //setup timer for monitoring session activity
    _monitorTimer = new QTimer(this);
    _monitorTimer->setSingleShot(true);
    connect(_monitorTimer, SIGNAL(timeout()), this, SLOT(monitorTimerDone()));

    // 定时更新term信息 => 目前为了更新标签标题信息
    _updateTimer = new QTimer(this);
    connect(_updateTimer, &QTimer::timeout, this, &Session::onUpdateTitleArgs);
    _updateTimer->start(500);
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
    if (_program.endsWith(QStringLiteral("/dash"))
            || _program.endsWith(QStringLiteral("/sh"))) {
        _updateTimer->setInterval(10);

        //用于更改updateTimer的时间间隔，防止CPU占用太高
        QTimer *monitorUpdateTimer = new QTimer(this);
        monitorUpdateTimer->setSingleShot(true);
        monitorUpdateTimer->start(500);
        connect(monitorUpdateTimer, &QTimer::timeout, this, [this] {
            if (_updateTimer->interval() < 500) {
                _updateTimer->setInterval(500);
            }
        });
    }
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
        connect(widget, SIGNAL(changedCursonPosition(int)), _emulation,
                SLOT(sendCursor(int)));
        // 先判断是否有远程连接,若有,则连接远程
        // connect(widget, SIGNAL(sendStringToEmu(const char *)), _emulation, SLOT(sendString(const char *)));

        // allow emulation to notify view when the foreground process
        // indicates whether or not it is interested in mouse signals
        connect( _emulation , SIGNAL(programUsesMouseChanged(bool)) , widget ,
                 SLOT(setUsesMouse(bool)) );
        connect( _emulation , &Konsole::Emulation::enableAlternateScrolling, widget, &Konsole::TerminalDisplay::setAlternateScrolling);

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

/*******************************************************************************
 1. @函数:    onUpdateTitleArgs
 2. @作者:    ut000610 戴正文
 3. @日期:    2020-12-02
 4. @说明:    更新标签标题参数
*******************************************************************************/
void Session::onUpdateTitleArgs()
{
    ProcessInfo *process = getProcessInfo();

    // format tab titles using process info
    bool ok = false;

    // 用户名 %u
    QString userName = process->userName();
    if (_userName != userName) {
        _userName = userName;
        emit titleArgsChange(QLatin1String("%u"), _userName);
    }

    //title.replace(QLatin1String("%h"), Konsole::ProcessInfo::localHost());
    // 程序名 %n
    QString programName = process->name(&ok);
    if (_programName != programName) {
        _programName = programName;
        emit titleArgsChange(QLatin1String("%n"), _programName);
    }


    // 获取当前目录 %D
    QString dir = _reportedWorkingUrl.toLocalFile();
    ok = true;
    if (dir.isEmpty()) {
        // update current directory from process
        updateWorkingDirectory();
        // Previous process may have been freed in updateSessionProcessInfo()
        process = getProcessInfo();
        dir = process->currentDir(&ok);
    }
    if (_currentDir != dir) {
        _currentDir = dir;
        emit titleArgsChange(QLatin1String("%D"), _currentDir);
    }
}

void Session::onPrimaryScreenInUse(bool use)
{
    _isPrimaryScreen = use;
    emit primaryScreenInUse(use);
}

void Session::removeView(TerminalDisplay *widget)
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
    QString programPath = exec;
    if (exec.startsWith(QLatin1Char('/')) || exec.isEmpty()) {
        const QString defaultShell{QLatin1String("/bin/bash")};

        QFile excheck(exec);
#if 0 // 下一期优化
        // exec是空的，或者文件不存在，则替换exec为$SHELL
        if (exec.isEmpty() || !excheck.exists()) {
            exec = QString::fromLocal8Bit(qgetenv("SHELL"));
        }
        excheck.setFileName(exec);
#endif
        // SHELL是空的，或者文件不存在，则替换exec为/bin/sh
        if (exec.isEmpty() || !excheck.exists()) {
            qWarning() << "Neither default shell nor $SHELL is set to a correct path. Fallback to" << defaultShell;
            exec = defaultShell;
        }

    }

    if (programPath != exec) {
        // 说明有文件不存在，取最后找到的文件进行替换
        emit shellWarningMessage(exec, true);
    }

    // _arguments sometimes contain ("") so isEmpty()
    // or count() does not work as expected...
    QString argsTmp(_arguments.join(QLatin1Char(' ')).trimmed());
    QStringList arguments;
    arguments << exec;
    if (argsTmp.length())
        arguments << _arguments;

    if (!_initialWorkingDir.isEmpty() && QDir(_initialWorkingDir).exists()) {
        _shellProcess->setWorkingDirectory(_initialWorkingDir);
    } else {
        _shellProcess->setWorkingDirectory(QDir::homePath());
    }

    _shellProcess->setFlowControlEnabled(_flowControl);
    _shellProcess->setErase(_emulation->eraseChar());

    // this is not strictly accurate use of the COLORFGBG variable.  This does not
    // tell the terminal exactly which colors are being used, but instead approximates
    // the color scheme as "black on white" or "white on black" depending on whether
    // the background color is deemed dark or not
    // fix bug#39147 终端使用vim编辑脚本，命令行终端的主题为跟随系统时无法清楚查看脚本内容
    // 移除该环境变量，因为终端的主题颜色会实时变化，不能通过设置环境变量的方式
    //QString backgroundColorHint = _hasDarkBackground ? QLatin1String("COLORFGBG=15;0") : QLatin1String("COLORFGBG=0;15");

    /* if we do all the checking if this shell exists then we use it ;)
     * Dont know about the arguments though.. maybe youll need some more checking im not sure
     * However this works on Arch and FreeBSD now.
     */
    int result = _shellProcess->start(exec,
                                      arguments,
                                      _environment,// << backgroundColorHint,
                                      windowId(),
                                      _addToUtmp);

    if (result < 0) {
        //qDebug() << "CRASHED! result: " << result<<arguments;
        QString processError = _shellProcess->errorString();
        processError = processError.mid(processError.indexOf(":") + 1).trimmed();
        if(!processError.isEmpty())
            processError = "(" +processError + ")";

        QString infoText = QString("There was an error creating the child process for this terminal. \n"
                 "Failed to execute child process \"%1\"%2!")
                .arg(exec)
                .arg(processError);
        sendText(infoText);
        _userTitle = QString::fromLatin1("Session crashed");
        emit titleChanged();
        emit shellWarningMessage(exec, false);
        qWarning() << _shellProcess->errorString();
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

    if (modified) {
        // 标签标题变化前更新标签标题参数
        onUpdateTitleArgs();
        // 标签标题有变化，发送信号通知terminal
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

    _notifiedActivity=false;
}

bool Session::isPrimaryScreen()
{
    return _isPrimaryScreen;
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
    if ( minLines > 0 && minColumns > 0 ) {
        _emulation->setImageSize( minLines , minColumns );
        _shellProcess->setWindowSize( minLines , minColumns );
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
    int result = ::kill(static_cast<pid_t>(_shellProcess->processId()),signal);

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
    //检测到当前命令是代码中通过sendText发给终端的(而不是用户手动输入的命令)
    bool isSendByRemoteManage = this->property("isSendByRemoteManage").toBool();
    if (isSendByRemoteManage) {
        //将isSendByRemoteManage标记同步给Pty
        _shellProcess->setProperty("isSendByRemoteManage", QVariant(true));
    }
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
    /**
    相关说明如下：
    1._wantedClose ：session是否正常结束，当session.close 或 session.析构时 为true，默认false
    2._autoClose   ：是否自动关闭 由传参控制，当传参含--keep-open 或 含-e 时 为false，默认true
    3.当【运行脚本命令-C】且【_autoClose=false】时，命令最后会补上exit，命令结束会触发process.finished-》session.done
    */
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
            infoText.sprintf("There was an error creating the child process for this terminal. \n"
                     "Failed to execute child process \"%s\"(No such file or directory)!", _program.toUtf8().data());
            message = "Session crashed.";
        }
        else {
            infoText.sprintf("The child process exited normally with status %d.", exitStatus);
            message.sprintf("Session '%s' exited with status %d.",
                      _nameTitle.toUtf8().data(), exitStatus);
        }
        _userTitle = message;
        //sendText(infoText);
        emit titleChanged();
    }
    else
    {
        sendText("\n");
        sendText(tr("Press any keys to exit"));
    }
    emit almostFinished();
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
    return _shellProcess->flowControlEnabled();
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
void Session::onReceiveBlock(const char * buf, int len, bool isCommandExec)
{
    _emulation->receiveData(buf, len, isCommandExec);
    emit receivedData( QString::fromLatin1( buf, len ) );
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
    return (_shellProcess->processId() != _shellProcess->foregroundProcessGroup());
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
    return static_cast<int>(_shellProcess->processId());
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

        connect( master->emulation() , SIGNAL(sendData(const char *, int, const QTextCodec *)) , other->emulation() ,
                 SLOT(sendString(const char *, int, const QTextCodec *)) );
    }
}
void SessionGroup::disconnectPair(Session * master , Session * other)
{
//    qDebug() << k_funcinfo;

    if ( _masterMode & CopyInputToAll ) {
        qDebug() << "Disconnecting session " << master->nameTitle() << "from" << other->nameTitle();

        disconnect( master->emulation() , SIGNAL(sendData(const char *, int, const QTextCodec *)) , other->emulation() ,
                    SLOT(sendString(const char *, int, const QTextCodec *)) );
    }
}

//#include "moc_Session.cpp"
