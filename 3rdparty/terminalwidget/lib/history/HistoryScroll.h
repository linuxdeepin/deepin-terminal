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

#ifndef HISTORYSCROLL_H
#define HISTORYSCROLL_H

// Konsole
#include "../Character.h"

// Qt
#include <QVector>

//#include "konsole_export.h"
#define KONSOLEPRIVATE_EXPORT

namespace Konsole
{
//////////////////////////////////////////////////////////////////////
// Abstract base class for file and buffer versions
//////////////////////////////////////////////////////////////////////
class HistoryType;

class HistoryScroll
{
public:
    explicit HistoryScroll(HistoryType *);
    virtual ~HistoryScroll();

    virtual bool hasScroll();

    // access to history
    virtual int  getLines() = 0;
    virtual int  getMaxLines() = 0;
    virtual int  getLineLen(int lineno) = 0;
    virtual void getCells(int lineno, int colno, int count, Character res[]) = 0;
    virtual bool isWrappedLine(int lineNumber) = 0;
    virtual LineProperty getLineProperty(int lineno) = 0;

    // adding lines.
    virtual void addCells(const Character a[], int count) = 0;
    // convenience method - this is virtual so that subclasses can take advantage
    // of QVector's implicit copying
    virtual void addCellsVector(const QVector<Character> &cells)
    {
        addCells(cells.data(), cells.size());
    }

    virtual void addLine(LineProperty lineProperty = 0) = 0;

    // modify history
    virtual void removeCells() = 0;
    virtual int reflowLines(int columns) = 0;
    //
    // FIXME:  Passing around constant references to HistoryType instances
    // is very unsafe, because those references will no longer
    // be valid if the history scroll is deleted.
    //
    const HistoryType &getType() const
    {
        return *_historyType;
    }

protected:
    HistoryType *_historyType;
    const int MAX_REFLOW_LINES = 20000;
};

}

#endif
