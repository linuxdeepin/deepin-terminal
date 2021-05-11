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

#ifndef COMPACTHISTORYSCROLL_H
#define COMPACTHISTORYSCROLL_H

#include "history/HistoryScroll.h"
#include "history/compact/CompactHistoryLine.h"

namespace Konsole
{

class CompactHistoryScroll : public HistoryScroll
{
    typedef QList<CompactHistoryLine *> HistoryArray;

public:
    explicit CompactHistoryScroll(unsigned int maxLineCount = 1000);
    ~CompactHistoryScroll() override;

    int  getLines() override;
    int  getMaxLines() override;
    int  getLineLen(int lineNumber) override;
    void getCells(int lineNumber, int startColumn, int count, Character buffer[]) override;
    bool isWrappedLine(int lineNumber) override;

    void addCells(const Character a[], int count) override;
    void addCellsVector(const TextLine &cells) override;
    void addLine(bool previousWrapped = false) override;

    void insertCellsVector(int position, const TextLine &cells) override;
    void insertCells(int position, const Character a[], int count) override;
    void removeCells(int position) override;

    void setMaxNbLines(unsigned int lineCount);
    void setCellsAt(int position, const Character a[], int count) override;
    void setCellsVectorAt(int position, const TextLine &cells) override;
    void setLineAt(int position, bool previousWrapped) override;

    int reflowLines(int columns) override;

private:
    bool hasDifferentColors(const TextLine &line) const;
    HistoryArray _lines;
    CompactHistoryBlockList _blockList;

    unsigned int _maxLineCount;
};

}

#endif
