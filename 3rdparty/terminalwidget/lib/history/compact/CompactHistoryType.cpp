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
#include "CompactHistoryType.h"

#include "CompactHistoryScroll.h"

using namespace Konsole;

// Reasonable line size
static const int LINE_SIZE = 1024;

CompactHistoryType::CompactHistoryType(unsigned int nbLines) :
    _maxLines(nbLines)
{
}

bool CompactHistoryType::isEnabled() const
{
    return true;
}

int CompactHistoryType::maximumLineCount() const
{
    return _maxLines;
}

HistoryScroll *CompactHistoryType::scroll(HistoryScroll *old) const
{
    if (old != nullptr) {
        auto *newBuffer = dynamic_cast<CompactHistoryScroll *>(old);
        if (newBuffer != nullptr) {
            newBuffer->setMaxNbLines(_maxLines);
            return newBuffer;
        }

        newBuffer = new CompactHistoryScroll(_maxLines);
        
        Character line[LINE_SIZE];
        int lines = old->getLines();
        int i = qMax((lines - (int)_maxLines - 1), 0);
        for (; i < lines; i++) {
            int size = old->getLineLen(i);
            if (size > LINE_SIZE) {
                auto tmp_line = new Character[size];
                old->getCells(i, 0, size, tmp_line);
                newBuffer->addCells(tmp_line, size);
                newBuffer->addLine(old->isWrappedLine(i));
                delete[] tmp_line;
            } else {
                old->getCells(i, 0, size, line);
                newBuffer->addCells(line, size);
                newBuffer->addLine(old->isWrappedLine(i));
            }
        }

        delete old;
        return newBuffer;
    }
    return new CompactHistoryScroll(_maxLines);
}
