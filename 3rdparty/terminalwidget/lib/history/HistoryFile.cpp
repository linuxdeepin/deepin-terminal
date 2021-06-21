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
#include "HistoryFile.h"
#include <QApplication>
#include <QStandardPaths>

// System
#include <cerrno>
#include <unistd.h>

// Qt
#include <QDir>
#include <QUrl>
#include <QDebug>

using namespace Konsole;

Q_GLOBAL_STATIC(QString, historyFileLocation)

// History File ///////////////////////////////////////////
HistoryFile::HistoryFile() :
    _length(0),
    _fileMap(nullptr),
    _readWriteBalance(0)
{
    // Determine the temp directory once
    // This class is called 3 times for each "unlimited" scrollback.
    // This has the down-side that users must restart to
    // load changes.
    if (!historyFileLocation.exists()) {
        QString fileLocation;
        // Validate file location
        const QFileInfo fi(fileLocation);
        if (fileLocation.isEmpty() || !fi.exists() || !fi.isDir() || !fi.isWritable()) {
            qDebug()<<"Invalid scrollback folder "<<fileLocation<<"; using " << QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
            // Per Qt docs, this path is never empty; not sure if that
            // means it always exists.
            fileLocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
            const QFileInfo fi2(fileLocation);
            if (!fi2.exists()) {
                if (!QDir().mkpath(fileLocation)) {
                    qDebug()<<"Unable to create scrollback folder "<<fileLocation;
                }
            }
        }
        *historyFileLocation() = fileLocation;
    }
    const QString tmpDir = *historyFileLocation();
    const QString tmpFormat = tmpDir + QLatin1Char('/') + QLatin1String("konsole-XXXXXX.history");
    _tmpFile.setFileTemplate(tmpFormat);
    if (_tmpFile.open()) {
#if defined(Q_OS_LINUX)
        qDebug("HistoryFile: /proc/%lld/fd/%d", qApp->applicationPid(), _tmpFile.handle());
#endif
        // On some systems QTemporaryFile creates unnamed file.
        // Do not interfere in such cases.
        if (_tmpFile.exists()) {
            // Remove file entry from filesystem. Since the file
            // is opened, it will still be available for reading
            // and writing. This guarantees the file won't remain
            // in filesystem after process termination, even when
            // there was a crash.
            unlink(QFile::encodeName(_tmpFile.fileName()).constData());
        }
    }
}

HistoryFile::~HistoryFile()
{
    if (_fileMap != nullptr) {
        unmap();
    }
}

//TODO:  Mapping the entire file in will cause problems if the history file becomes exceedingly large,
//(ie. larger than available memory).  HistoryFile::map() should only map in sections of the file at a time,
//to avoid this.
void HistoryFile::map()
{
    Q_ASSERT(_fileMap == nullptr);

    if (_tmpFile.flush()) {
        Q_ASSERT(_tmpFile.size() >= _length);
        _fileMap = _tmpFile.map(0, _length);
    }

    //if mmap'ing fails, fall back to the read-lseek combination
    if (_fileMap == nullptr) {
        _readWriteBalance = 0;
        //qDebug() << "mmap'ing history failed.  errno = " << errno;
    }
}

void HistoryFile::unmap()
{
    Q_ASSERT(_fileMap != nullptr);

    if (_tmpFile.unmap(_fileMap)) {
        _fileMap = nullptr;
    }

    Q_ASSERT(_fileMap == nullptr);

}

void HistoryFile::add(const char *buffer, qint64 count)
{
    if (_fileMap != nullptr) {
        unmap();
    }

    if (_readWriteBalance < INT_MAX) {
        _readWriteBalance++;
    }

    qint64 rc = 0;

    if (!_tmpFile.seek(_length)) {
        perror("HistoryFile::add.seek");
        return;
    }
    rc = _tmpFile.write(buffer, count);
    if (rc < 0) {
        perror("HistoryFile::add.write");
        return;
    }
    _length += rc;
}

void HistoryFile::get(char *buffer, qint64 size, qint64 loc)
{

    if (loc < 0 || size < 0 || loc + size > _length) {
        fprintf(stderr, "getHist(...,%lld,%lld): invalid args.\n", size, loc);
        return;
    }

    //count number of get() calls vs. number of add() calls.
    //If there are many more get() calls compared with add()
    //calls (decided by using MAP_THRESHOLD) then mmap the log
    //file to improve performance.
    if (_readWriteBalance > INT_MIN) {
        _readWriteBalance--;
    }
    if ((_fileMap == nullptr) && _readWriteBalance < MAP_THRESHOLD) {
        map();
    }

    if (_fileMap != nullptr) {
        memcpy(buffer, _fileMap + loc, size);
    } else {
        qint64 rc = 0;

        if (!_tmpFile.seek(loc)) {
            perror("HistoryFile::get.seek");
            return;
        }
        rc = _tmpFile.read(buffer, size);
        if (rc < 0) {
            perror("HistoryFile::get.read");
            return;
        }
    }
}

void HistoryFile::removeLast(qint64 loc)
{
    if (loc < 0 || loc > _length) {
        fprintf(stderr, "removeLast(%lld): invalid args.\n", loc);
        return;
    }
    _length = loc;
}

qint64 HistoryFile::len() const
{
    return _length;
}
