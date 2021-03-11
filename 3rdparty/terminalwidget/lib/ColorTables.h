/*
    This file is part of Konsole, an X terminal.

    Copyright 2006-2008 by Robert Knight <robertknight@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

#ifndef _COLOR_TABLE_H
#define _COLOR_TABLE_H

#include "CharacterColor.h"

//using namespace Konsole;
#if 0
static const ColorEntry whiteonblack_color_table[TABLE_COLORS] = {
    // normal
    ColorEntry(QColor(0xFF,0xFF,0xFF), false ), ColorEntry( QColor(0x00,0x00,0x00), true ), // Dfore, Dback
    ColorEntry(QColor(0x00,0x00,0x00), false ), ColorEntry( QColor(0xB2,0x18,0x18), false ), // Black, Red
    ColorEntry(QColor(0x18,0xB2,0x18), false ), ColorEntry( QColor(0xB2,0x68,0x18), false ), // Green, Yellow
    ColorEntry(QColor(0x18,0x18,0xB2), false ), ColorEntry( QColor(0xB2,0x18,0xB2), false ), // Blue, Magenta
    ColorEntry(QColor(0x18,0xB2,0xB2), false ), ColorEntry( QColor(0xB2,0xB2,0xB2), false ), // Cyan, White
    // intensiv
    ColorEntry(QColor(0x00,0x00,0x00), false ), ColorEntry( QColor(0xFF,0xFF,0xFF), true ),
    ColorEntry(QColor(0x68,0x68,0x68), false ), ColorEntry( QColor(0xFF,0x54,0x54), false ),
    ColorEntry(QColor(0x54,0xFF,0x54), false ), ColorEntry( QColor(0xFF,0xFF,0x54), false ),
    ColorEntry(QColor(0x54,0x54,0xFF), false ), ColorEntry( QColor(0xFF,0x54,0xFF), false ),
    ColorEntry(QColor(0x54,0xFF,0xFF), false ), ColorEntry( QColor(0xFF,0xFF,0xFF), false )
};

static const ColorEntry greenonblack_color_table[TABLE_COLORS] = {
    ColorEntry(QColor(    24, 240,  24),  false), ColorEntry(QColor(     0,   0,   0),  true),
    ColorEntry(QColor(     0,   0,   0),  false), ColorEntry(QColor(   178,  24,  24),  false),
    ColorEntry(QColor(    24, 178,  24),  false), ColorEntry(QColor(   178, 104,  24),  false),
    ColorEntry(QColor(    24,  24, 178),  false), ColorEntry(QColor(   178,  24, 178),  false),
    ColorEntry(QColor(    24, 178, 178),  false), ColorEntry(QColor(   178, 178, 178),  false),
    // intensive colors
    ColorEntry(QColor(   24, 240,  24),  false ), ColorEntry(QColor(    0,   0,   0),  true ),
    ColorEntry(QColor(  104, 104, 104),  false ), ColorEntry(QColor(  255,  84,  84),  false ),
    ColorEntry(QColor(   84, 255,  84),  false ), ColorEntry(QColor(  255, 255,  84),  false ),
    ColorEntry(QColor(   84,  84, 255),  false ), ColorEntry(QColor(  255,  84, 255),  false ),
    ColorEntry(QColor(   84, 255, 255),  false ), ColorEntry(QColor(  255, 255, 255),  false )
};

static const ColorEntry blackonlightyellow_color_table[TABLE_COLORS] = {
    ColorEntry(QColor(  0,   0,   0),  false),  ColorEntry(QColor( 255, 255, 221),  true),
    ColorEntry(QColor(  0,   0,   0),  false),  ColorEntry(QColor( 178,  24,  24),  false),
    ColorEntry(QColor( 24, 178,  24),  false),  ColorEntry(QColor( 178, 104,  24),  false),
    ColorEntry(QColor( 24,  24, 178),  false),  ColorEntry(QColor( 178,  24, 178),  false),
    ColorEntry(QColor( 24, 178, 178),  false),  ColorEntry(QColor( 178, 178, 178),  false),
    ColorEntry(QColor(  0,   0,   0),  false),  ColorEntry(QColor( 255, 255, 221),  true),
    ColorEntry(QColor(104, 104, 104),  false),  ColorEntry(QColor( 255,  84,  84),  false),
    ColorEntry(QColor( 84, 255,  84),  false),  ColorEntry(QColor( 255, 255,  84),  false),
    ColorEntry(QColor( 84,  84, 255),  false),  ColorEntry(QColor( 255,  84, 255),  false),
    ColorEntry(QColor( 84, 255, 255),  false),  ColorEntry(QColor( 255, 255, 255),  false)
};


#endif


#endif

