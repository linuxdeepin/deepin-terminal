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
#include "HistoryScrollNone.h"

#include "HistoryTypeNone.h"

using namespace Konsole;

// History Scroll None //////////////////////////////////////

HistoryScrollNone::HistoryScrollNone() :
    HistoryScroll(new HistoryTypeNone())
{
}

HistoryScrollNone::~HistoryScrollNone()
{
}

bool HistoryScrollNone::hasScroll()
{
    return false;
}

int HistoryScrollNone::getLines()
{
    return 0;
}

int HistoryScrollNone::getMaxLines()
{
    return 0;
}

int HistoryScrollNone::getLineLen(int)
{
    return 0;
}

bool HistoryScrollNone::isWrappedLine(int /*lineno*/)
{
    return false;
}

LineProperty HistoryScrollNone::getLineProperty(int /*lineno*/)
{
    return 0;
}

void HistoryScrollNone::getCells(int, int, int, Character [])
{
}

void HistoryScrollNone::addCells(const Character [], int)
{
}

void HistoryScrollNone::removeCells()
{
}

int HistoryScrollNone::reflowLines(int)
{
    return 0;
}
