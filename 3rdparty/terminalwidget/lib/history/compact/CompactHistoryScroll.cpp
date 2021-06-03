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
#include "CompactHistoryScroll.h"

#include "CompactHistoryType.h"

using namespace Konsole;

CompactHistoryScroll::CompactHistoryScroll(unsigned int maxLineCount) :
    HistoryScroll(new CompactHistoryType(maxLineCount)),
    _lines(),
    _blockList(),
    _maxLineCount(0)
{
    ////qDebug() << "scroll of length " << maxLineCount << " created";
    setMaxNbLines(maxLineCount);
}

CompactHistoryScroll::~CompactHistoryScroll()
{
    qDeleteAll(_lines.begin(), _lines.end());
    _lines.clear();
}

void CompactHistoryScroll::addCellsVector(const TextLine &cells)
{
    CompactHistoryLine *line;
    line = new (_blockList) CompactHistoryLine(cells, _blockList);

    _lines.append(line);

    if (_lines.size() > static_cast<int>(_maxLineCount)) {
        delete _lines.takeAt(0);
    }
}

void CompactHistoryScroll::addCells(const Character a[], int count)
{
    TextLine newLine(count);
    std::copy(a, a + count, newLine.begin());
    addCellsVector(newLine);
}

void CompactHistoryScroll::addLine(bool previousWrapped)
{
    CompactHistoryLine *line = _lines.last();
    ////qDebug() << "last line at address " << line;
    line->setWrapped(previousWrapped);
}

int CompactHistoryScroll::getLines()
{
    return _lines.size();
}

int CompactHistoryScroll::getMaxLines()
{
    return static_cast<int>(_maxLineCount);
}

int CompactHistoryScroll::getLineLen(int lineNumber)
{
    if ((lineNumber < 0) || (lineNumber >= _lines.size())) {
        //qDebug() << "requested line invalid: 0 < " << lineNumber << " < " <<_lines.size();
        //Q_ASSERT(lineNumber >= 0 && lineNumber < _lines.size());
        return 0;
    }
    CompactHistoryLine *line = _lines[lineNumber];
    ////qDebug() << "request for line at address " << line;
    return line->getLength();
}

void CompactHistoryScroll::getCells(int lineNumber, int startColumn, int count, Character buffer[])
{
    if (count == 0) {
        return;
    }
    Q_ASSERT(lineNumber < _lines.size());
    CompactHistoryLine *line = _lines[lineNumber];
    Q_ASSERT(startColumn >= 0);
    Q_ASSERT(static_cast<unsigned int>(startColumn) <= line->getLength() - count);
    line->getCharacters(buffer, count, startColumn);
}

void CompactHistoryScroll::setMaxNbLines(unsigned int lineCount)
{
    _maxLineCount = lineCount;

    while (_lines.size() > static_cast<int>(lineCount)) {
        delete _lines.takeAt(0);
    }
    ////qDebug() << "set max lines to: " << _maxLineCount;
}

void CompactHistoryScroll::insertCellsVector(int position, const TextLine &cells)
{
    CompactHistoryLine *line = new (_blockList) CompactHistoryLine(cells, _blockList);

    _lines.insert(position, line);

    if (_lines.size() > static_cast<int>(_maxLineCount)) {
        delete _lines.takeAt(0);
    }
}

void CompactHistoryScroll::insertCells(int position, const Character a[], int count)
{
    TextLine newLine(count);
    std::copy(a, a + count, newLine.begin());
    insertCellsVector(position, newLine);
}

void CompactHistoryScroll::removeCells(int position)
{
    delete _lines.takeAt(position);
}

void CompactHistoryScroll::setCellsAt(int position, const Character a[], int count)
{
    TextLine newLine(count);
    std::copy(a, a + count, newLine.begin());
    setCellsVectorAt(position, newLine);
}

void CompactHistoryScroll::setCellsVectorAt(int position, const TextLine &cells)
{
    CompactHistoryLine *line = new (_blockList) CompactHistoryLine(cells, _blockList);

    delete _lines.takeAt(position);
    _lines.insert(position, line);
}

void CompactHistoryScroll::setLineAt(int position, bool previousWrapped)
{
    CompactHistoryLine *line = _lines.at(position);

    line->setWrapped(previousWrapped);
}

bool CompactHistoryScroll::isWrappedLine(int lineNumber)
{
    Q_ASSERT(lineNumber < _lines.size());
    return _lines[lineNumber]->isWrapped();
}

int CompactHistoryScroll::reflowLines(int columns)
{
    auto getCharacterBuffer = [](int size) {
        static QVector<Character> characterBuffer(1024);
        if (characterBuffer.count() < size) {
            characterBuffer.resize(size);
        }

        return characterBuffer.data();
    };

    // Join the line and move the data to next line if needed
    int removedLines = 0;
    int currentPos = 0;
    if (getLines() > MAX_REFLOW_LINES) {
        currentPos = getLines() - MAX_REFLOW_LINES;
    }
    while (currentPos < getLines()) {
        int curr_linelen = getLineLen(currentPos);
        // Join wrapped line in current history position
        if (isWrappedLine(currentPos) && currentPos < getLines() - 1) {
            int next_linelen = getLineLen(currentPos + 1);
            auto *new_line = getCharacterBuffer(curr_linelen + next_linelen);
            bool new_line_property = isWrappedLine(currentPos + 1);

            // Join the lines
            getCells(currentPos, 0, curr_linelen, new_line);
            getCells(currentPos + 1, 0, next_linelen, new_line + curr_linelen);

            // save the new_line in history and remove the next line
            setCellsAt(currentPos, new_line, curr_linelen + next_linelen);
            setLineAt(currentPos, new_line_property);
            removeCells(currentPos + 1);
            continue;
        }

        // if the current line > columns it will need a new line
        if (curr_linelen > columns) {
            bool removeLine = getLines() == getMaxLines();
            auto *curr_line = getCharacterBuffer(curr_linelen);
            bool curr_line_property = isWrappedLine(currentPos);
            getCells(currentPos, 0, curr_linelen, curr_line);

            setCellsAt(currentPos, curr_line, columns);
            setLineAt(currentPos, true);
            if (currentPos < getMaxLines() - 1) {
                int correctPosition = (getLines() == getMaxLines()) ? 0 : 1;
                insertCells(currentPos + 1, curr_line + columns, curr_linelen - columns);
                setLineAt(currentPos + correctPosition, curr_line_property);
            } else {
                addCells(curr_line + columns, curr_linelen - columns);
                addLine(curr_line_property);
                currentPos--;
            }
            if (removeLine) {
                removedLines += 1;
            }
        }
        currentPos++;
    }
    return removedLines;
}

