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

#ifndef _KONSOLE_WCWIDTH_H_
#define _KONSOLE_WCWIDTH_H_

// Standard
#include <string>

// Qt
#include <QtGlobal>
#include <QPainter>

#ifdef UNIT_TEST
    #define STATIC_METHOD
    #define INLINE_METHOD
#else
    #define STATIC_METHOD static
    #define INLINE_METHOD inline
#endif

int konsole_wcwidth(wchar_t ucs);

int string_width( const std::wstring & wstr );

namespace Konsole {

/**
 * Helper functions for drawing characters from "Box Drawing" and "Block Elements" Unicode blocks.
 */
namespace LineBlockCharacters {

#ifdef UNIT_TEST
    bool drawBasicLineCharacter(QPainter& paint, int x, int y, int w, int h, uchar code,
                                       bool bold);

    uint lineWidth(uint fontWidth, bool heavy, bool bold);

    bool drawDashedLineCharacter(QPainter &paint, int x, int y, int w, int h, uchar code,
                                               bool bold);
    bool drawRoundedCornerLineCharacter(QPainter &paint, int x, int y, int w, int h,
                                                      uchar code, bool bold);
    bool drawDiagonalLineCharacter(QPainter &paint, int x, int y, int w, int h,
                                                 uchar code, bool bold);
    bool drawBlockCharacter(QPainter &paint, int x, int y, int w, int h, uchar code,
                                          bool bold);
#endif

    /**
     * Returns true if the character can be drawn by draw() function.
     *
     * @param ucs4cp Character to test's UCS4 code point
     */
    inline static bool canDraw(uint ucs4cp) {
        return (0x2500 <= ucs4cp && ucs4cp <= 0x259F);
    }

    /**
     * Draws character.
     *
     * @param paint QPainter to draw on
     * @param cellRect Rectangle to draw in
     * @param chr Character to be drawn
     * @param bold Whether the character should be boldface
     */
    void draw(QPainter &paint, const QRect &cellRect, const QChar &chr, bool bold);

} // namespace LineBlockCharacters
} // namespace Konsole

int characterWidth(uint ucs4);

#endif
