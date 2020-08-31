/* $XFree86: xc/programs/xterm/wcwidth.h,v 1.2 2001/06/18 19:09:27 dickey Exp $ */

/* Markus Kuhn -- 2001-01-12 -- public domain */
/* Adaptions for KDE by Waldo Bastian <bastian@kde.org> */
/*
    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>
*/


#ifndef _KONSOLE_WCWIDTH_H_
#define _KONSOLE_WCWIDTH_H_

// Standard
#include <string>

// Qt
#include <QtGlobal>
#include <QPainter>

int konsole_wcwidth(wchar_t ucs);

int string_width( const std::wstring & wstr );
namespace Konsole {

/**
 * Helper functions for drawing characters from "Box Drawing" and "Block Elements" Unicode blocks.
 */
namespace LineBlockCharacters {

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
