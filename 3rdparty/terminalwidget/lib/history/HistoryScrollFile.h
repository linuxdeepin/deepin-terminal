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

#ifndef HISTORYSCROLLFILE_H
#define HISTORYSCROLLFILE_H

// History
#include "HistoryFile.h"
#include "HistoryScroll.h"

namespace Konsole
{

//////////////////////////////////////////////////////////////////////
// File-based history (e.g. file log, no limitation in length)
//////////////////////////////////////////////////////////////////////

class HistoryScrollFile : public HistoryScroll
{
public:
    explicit HistoryScrollFile();
    ~HistoryScrollFile() override;

    int  getLines() override;
    int  getMaxLines() override;
    int  getLineLen(int lineno) override;
    void getCells(int lineno, int colno, int count, Character res[]) override;
    bool isWrappedLine(int lineno) override;
    LineProperty getLineProperty(int lineno) override;

    void addCells(const Character text[], int count) override;
    void addLine(LineProperty lineProperty = 0) override;

    // Modify history
    void removeCells() override;
    int reflowLines(int columns) override;

private:
    qint64 startOfLine(int lineno);

    HistoryFile _index; // lines Row(qint64)
    HistoryFile _cells; // text  Row(Character)
    HistoryFile _lineflags; // flags Row(unsigned char)

    struct reflowData { // data to reflow lines
        qint64 index;
        bool lineFlag;
    };
};

}

#endif
