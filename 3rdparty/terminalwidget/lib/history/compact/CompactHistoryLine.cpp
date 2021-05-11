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


#include "CompactHistoryLine.h"

using namespace Konsole;

void *CompactHistoryLine::operator new(size_t size, CompactHistoryBlockList &blockList)
{
    return blockList.allocate(size);
}

void CompactHistoryLine::operator delete(void *)
{
    /* do nothing, deallocation from pool is done in destructor*/
}

CompactHistoryLine::CompactHistoryLine(const TextLine &line, CompactHistoryBlockList &bList) :
    _blockListRef(bList),
    _formatArray(nullptr),
    _text(nullptr),
    _formatLength(0),
    _wrapped(false)
{
    if (!line.isEmpty()) {
        _length = line.size();

        // count number of different formats in this text line
        Character c = line[0];
        _formatLength = 1;
        for (auto &k : line) {
            if (!(k.equalsFormat(c))) {
                _formatLength++; // format change detected
                c = k;
            }
        }

        _formatArray = static_cast<CharacterFormat *>(_blockListRef.allocate(sizeof(CharacterFormat) * _formatLength));
        Q_ASSERT(_formatArray != nullptr);
        _text = static_cast<uint *>(_blockListRef.allocate(sizeof(uint) * line.size()));
        Q_ASSERT(_text != nullptr);

        c = line[0];
        _formatArray[0].setFormat(c);
        _formatArray[0].startPos = 0;
        for (int i = 0, k = 1; i < _length; i++) {
            if (!line[i].equalsFormat(c)) {
                c = line[i];
                _formatArray[k].setFormat(c);
                _formatArray[k].startPos = i;
                k++;
            }
            _text[i] = line[i].character;
        }

        _wrapped = false;
    }
}

CompactHistoryLine::~CompactHistoryLine()
{
    if (_length > 0) {
        _blockListRef.deallocate(_text);
        _blockListRef.deallocate(_formatArray);
    }
    _blockListRef.deallocate(this);
}

void CompactHistoryLine::getCharacters(Character *array, int size, int startColumn)
{
    Q_ASSERT(startColumn >= 0 && size >= 0);
    Q_ASSERT(startColumn + size <= static_cast<int>(getLength()));

    int formatPos = 0;
    while ((formatPos + 1) < _formatLength && startColumn >= _formatArray[formatPos + 1].startPos) {
        formatPos++;
    }

    for (int i = startColumn; i < size + startColumn; i++) {
        if ((formatPos + 1) < _formatLength && i == _formatArray[formatPos + 1].startPos) {
            formatPos++;
        }
        array[i - startColumn] = Character(_text[i], 
                                           _formatArray[formatPos].fgColor, 
                                           _formatArray[formatPos].bgColor, 
                                           _formatArray[formatPos].rendition);
    }
}

bool CompactHistoryLine::isWrapped() const
{
    return _wrapped;
}

void CompactHistoryLine::setWrapped(bool value)
{
    _wrapped = value;
}

unsigned int CompactHistoryLine::getLength() const
{
    return _length;
}
