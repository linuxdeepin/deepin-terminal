/*
    Copyright 2007-2008 by Robert Knight <robertknight@gmail.countm>

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
#include "ProcessInfo.h"

// Unix
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/param.h>
#include <errno.h>

// Qt
#include <QDir>
#include <QFileInfo>
#include <QFlags>
#include <QTextStream>
#include <QStringList>
#include <QSet>
#include <QDebug>

using namespace Konsole;

ProcessInfo::ProcessInfo(int pid) :
    _fields(ARGUMENTS),
    // arguments
    // are currently always valid,
    // they just return an empty
    // vector / map respectively
    // if no arguments
    // have been explicitly set
    _pid(pid),
    _parentPid(0),
    _foregroundPid(0),
    _userId(0),
    _lastError(NoError),
    _name(QString()),
    _userName(QString()),
    _userHomeDir(QString()),
    _currentDir(QString()),
    _userNameRequired(true),
    _arguments(QVector<QString>())
{
}

ProcessInfo::~ProcessInfo()
{
}

ProcessInfo::Error ProcessInfo::error() const
{
    return _lastError;
}

void ProcessInfo::setError(Error error)
{
    _lastError = error;
}

void ProcessInfo::update()
{
    readCurrentDir(_pid);
}

QString ProcessInfo::validCurrentDir() const
{
    bool ok = false;

    // read current dir, if an error occurs try the parent as the next
    // best option
    int currentPid = parentPid(&ok);
    QString dir = currentDir(&ok);
    while (!ok && currentPid != 0) {
        ProcessInfo *current = ProcessInfo::newInstance(currentPid, QString());
        current->update();
        currentPid = current->parentPid(&ok);
        dir = current->currentDir(&ok);
        delete current;
    }

    return dir;
}

QVector<QString> ProcessInfo::arguments(bool *ok) const
{
    *ok = _fields.testFlag(ARGUMENTS);

    return _arguments;
}

bool ProcessInfo::isValid() const
{
    return _fields.testFlag(PROCESS_ID);
}

int ProcessInfo::pid(bool *ok) const
{
    *ok = _fields.testFlag(PROCESS_ID);

    return _pid;
}

int ProcessInfo::parentPid(bool *ok) const
{
    *ok = _fields.testFlag(PARENT_PID);

    return _parentPid;
}

int ProcessInfo::foregroundPid(bool *ok) const
{
    *ok = _fields.testFlag(FOREGROUND_PID);

    return _foregroundPid;
}

QString ProcessInfo::name(bool *ok) const
{
    *ok = _fields.testFlag(NAME);

    return _name;
}

int ProcessInfo::userId(bool *ok) const
{
    *ok = _fields.testFlag(UID);

    return _userId;
}

QString ProcessInfo::userName() const
{
    return _userName;
}

QString ProcessInfo::userHomeDir() const
{
    return _userHomeDir;
}

void ProcessInfo::setPid(int pid)
{
    _pid = pid;
    _fields |= PROCESS_ID;
}

void ProcessInfo::setUserId(int uid)
{
    _userId = uid;
    _fields |= UID;
}

void ProcessInfo::setUserName(const QString &name)
{
    _userName = name;
    setUserHomeDir();
}

void ProcessInfo::setUserHomeDir()
{
    _userHomeDir = QDir::homePath();
}

void ProcessInfo::setParentPid(int pid)
{
    _parentPid = pid;
    _fields |= PARENT_PID;
}

void ProcessInfo::setForegroundPid(int pid)
{
    _foregroundPid = pid;
    _fields |= FOREGROUND_PID;
}

void ProcessInfo::setUserNameRequired(bool need)
{
    _userNameRequired = need;
}

bool ProcessInfo::userNameRequired() const
{
    return _userNameRequired;
}

QString ProcessInfo::currentDir(bool *ok) const
{
    if (ok != nullptr) {
        *ok = (_fields & CURRENT_DIR) != 0;
    }

    return _currentDir;
}

void ProcessInfo::setCurrentDir(const QString &dir)
{
    _fields |= CURRENT_DIR;
    _currentDir = dir;
}

void ProcessInfo::setName(const QString &name)
{
    _name = name;
    _fields |= NAME;
}

void ProcessInfo::addArgument(const QString &argument)
{
    _arguments << argument;
}

void ProcessInfo::clearArguments()
{
    _arguments.clear();
}

void ProcessInfo::setFileError(QFile::FileError error)
{
    switch (error) {
    case QFile::PermissionsError:
        setError(ProcessInfo::PermissionsError);
        break;
    case QFile::NoError:
        setError(ProcessInfo::NoError);
        break;
    default:
        setError(ProcessInfo::UnknownError);
    }
}

UnixProcessInfo::UnixProcessInfo(int pid, const QString &titleFormat) :
    ProcessInfo(pid)
{
    // 目前的机制是需要用户名的，但是没有用到库里的titleFormat用的我们自己的机制
    Q_UNUSED(titleFormat);
    // setUserNameRequired(titleFormat.contains(QLatin1String("%u")));
}

void UnixProcessInfo::readProcessInfo(int pid)
{
    // prevent _arguments from growing longer and longer each time this
    // method is called.
    clearArguments();

    if (readProcInfo(pid)) {
        readArguments(pid);
        readCurrentDir(pid);
    }
}

void UnixProcessInfo::readUserName()
{
    bool ok = false;
    const int uid = userId(&ok);
    if (!ok) {
        return;
    }

    struct passwd passwdStruct;
    struct passwd *getpwResult;
    char *getpwBuffer;
    long getpwBufferSize;
    int getpwStatus;

    getpwBufferSize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (getpwBufferSize == -1) {
        getpwBufferSize = 16384;
    }

    getpwBuffer = new char[static_cast<unsigned>(getpwBufferSize)];
    if (getpwBuffer == nullptr) {
        return;
    }
    getpwStatus = getpwuid_r(static_cast<__uid_t>(uid), &passwdStruct, getpwBuffer, static_cast<size_t>(getpwBufferSize), &getpwResult);
    if ((getpwStatus == 0) && (getpwResult != nullptr)) {
        setUserName(QLatin1String(passwdStruct.pw_name));
    } else {
        setUserName(QString());
        qWarning() << "getpwuid_r returned error : " << getpwStatus;
    }
    delete [] getpwBuffer;
}

LinuxProcessInfo::LinuxProcessInfo(int pid, const QString &titleFormat) :
    UnixProcessInfo(pid, titleFormat)
{
}

bool LinuxProcessInfo::readCurrentDir(int pid)
{
    char path_buffer[MAXPATHLEN + 1];
    path_buffer[MAXPATHLEN] = 0;
    QByteArray procCwd = QFile::encodeName(QStringLiteral("/proc/%1/cwd").arg(pid));
    const int length = static_cast<int>(readlink(procCwd.constData(), path_buffer, MAXPATHLEN));
    if (length == -1)
    {
        setError(UnknownError);
        return false;
    }

    path_buffer[length] = '\0';
    QString path = QFile::decodeName(path_buffer);

    setCurrentDir(path);
    return true;
}

bool LinuxProcessInfo::readProcInfo(int pid)
{
    // indicies of various fields within the process status file which
    // contain various information about the process
    const int PARENT_PID_FIELD = 3;
    const int PROCESS_NAME_FIELD = 1;
    const int GROUP_PROCESS_FIELD = 7;

    QString parentPidString;
    QString processNameString;
    QString foregroundPidString;
    QString uidLine;
    QString uidString;
    QStringList uidStrings;

    // For user id read process status file ( /proc/<pid>/status )
    //  Can not use getuid() due to it does not work for 'su'
    QFile statusInfo(QStringLiteral("/proc/%1/status").arg(pid));
    if (statusInfo.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&statusInfo);
        QString statusLine;
        do {
            statusLine = stream.readLine();
            if (statusLine.startsWith(QLatin1String("Uid:"))) {
                uidLine = statusLine;
            }
        } while (!statusLine.isNull() && uidLine.isNull());

        uidStrings << uidLine.split(QLatin1Char('\t'), QString::SkipEmptyParts);
        // Must be 5 entries: 'Uid: %d %d %d %d' and
        // uid string must be less than 5 chars (uint)
        if (uidStrings.size() == 5) {
            uidString = uidStrings[1];
        }
        if (uidString.size() > 5) {
            uidString.clear();
        }

        bool ok = false;
        const int uid = uidString.toInt(&ok);
        if (ok) {
            setUserId(uid);
        }
        // This will cause constant opening of /etc/passwd
        if (userNameRequired()) {
            readUserName();
        }
    }
    else
    {
        setFileError(statusInfo.error());
        return false;
    }

    // read process status file ( /proc/<pid/stat )
    //
    // the expected file format is a list of fields separated by spaces, using
    // parenthesies to escape fields such as the process name which may itself contain
    // spaces:
    //
    // FIELD FIELD (FIELD WITH SPACES) FIELD FIELD
    //
    QFile processInfo(QStringLiteral("/proc/%1/stat").arg(pid));
    if (processInfo.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&processInfo);
        const QString &data = stream.readAll();

        int stack = 0;
        int field = 0;
        int pos = 0;

        while (pos < data.count()) {
            QChar c = data[pos];

            if (c == QLatin1Char('(')) {
                stack++;
            } else if (c == QLatin1Char(')')) {
                stack--;
            } else if (stack == 0 && c == QLatin1Char(' ')) {
                field++;
            } else {
                switch (field) {
                case PARENT_PID_FIELD:
                    parentPidString.append(c);
                    break;
                case PROCESS_NAME_FIELD:
                    processNameString.append(c);
                    break;
                case GROUP_PROCESS_FIELD:
                    foregroundPidString.append(c);
                    break;
                }
            }

            pos++;
        }
    }
    else
    {
        setFileError(processInfo.error());
        return false;
    }

    // check that data was read successfully
    bool ok = false;
    const int foregroundPid = foregroundPidString.toInt(&ok);
    if (ok)
    {
        setForegroundPid(foregroundPid);
    }

    const int parentPid = parentPidString.toInt(&ok);
    if (ok)
    {
        setParentPid(parentPid);
    }

    if (!processNameString.isEmpty())
    {
        setName(processNameString);
    }

    // update object state
    setPid(pid);

    return ok;
}

bool LinuxProcessInfo::readArguments(int pid)
{
    // read command-line arguments file found at /proc/<pid>/cmdline
    // the expected format is a list of strings delimited by null characters,
    // and ending in a double null character pair.

    QFile argumentsFile(QStringLiteral("/proc/%1/cmdline").arg(pid));
    if (argumentsFile.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&argumentsFile);
        const QString &data = stream.readAll();

        const QStringList &argList = data.split(QLatin1Char('\0'));

        for (int i = 0; i < argList.size(); i++) {
            const QString &entry = argList.at(i);
            if (!entry.isEmpty()) {
                addArgument(entry);
            }
        }
    }
    else
    {
        setFileError(argumentsFile.error());
    }

    return true;
}

//
// ProcessInfo::newInstance() is way at the bottom so it can see all of the
// implementations of the UnixProcessInfo abstract class.
//
ProcessInfo *ProcessInfo::newInstance(int pid, const QString &titleFormat)
{
    ProcessInfo *info;
    info = new LinuxProcessInfo(pid, titleFormat);
    info->readProcessInfo(pid);
    return info;
}
