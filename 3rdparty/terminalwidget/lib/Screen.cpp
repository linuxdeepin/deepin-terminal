/*
   This file is part of Konsole, an X terminal.

   Copyright 2007-2008 by Robert Knight <robert.knight@gmail.com>
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
#include "Screen.h"

// Standard
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <cctype>

// Qt
#include <QTextStream>
#include <QDate>
#include <QDebug>

// KDE
//#include <kdebug.h>

// Konsole
#include "konsole_wcwidth.h"
#include "Session.h"
#include "SessionManager.h"
#include "TerminalCharacterDecoder.h"
#include "TerminalDisplay.h"
#include "history/HistoryType.h"
#include "history/HistoryScrollNone.h"
#include "EscapeSequenceUrlExtractor.h"

using namespace Konsole;

//FIXME: this is emulation specific. Use false for xterm, true for ANSI.
//FIXME: see if we can get this from terminfo.
#define BS_CLEARS false

//Macro to convert x,y position on screen to position within an image.
//
//Originally the image was stored as one large contiguous block of
//memory, so a position within the image could be represented as an
//offset from the beginning of the block.  For efficiency reasons this
//is no longer the case.
//Many internal parts of this class still use this representation for parameters and so on,
//notably moveImage() and clearImage().
//This macro converts from an X,Y position into an image offset.
#ifndef loc
#define loc(X,Y) ((Y)*_columns+(X))
#endif


const Character Screen::DefaultChar = Character(' ',
                                      CharacterColor(COLOR_SPACE_DEFAULT,DEFAULT_FORE_COLOR),
                                      CharacterColor(COLOR_SPACE_DEFAULT,DEFAULT_BACK_COLOR),
                                      DEFAULT_RENDITION);

//#define REVERSE_WRAPPED_LINES  // for wrapped line debug

Screen::Screen(int l, int c)
    : _lines(l)
    , _columns(c)
    , _screenLines(_lines + 1)
    , _screenLinesSize(_lines)
    , _scrolledLines(0)
    , _droppedLines(0)
    , _history(new HistoryScrollNone())
    , _cuX(0)
    , _cuY(0)
    , _currentRendition(0)
    , _topMargin(0)
    , _bottomMargin(0)
    , _selBegin(0)
    , _selTopLeft(0)
    , _selBottomRight(0)
    , _blockSelectionMode(false)
    , _effectiveForeground(CharacterColor())
    , _effectiveBackground(CharacterColor())
    , _effectiveRendition(0)
    , _lastPos(-1)
    , _lastDrawnChar(0)
    , _escapeSequenceUrlExtractor(new EscapeSequenceUrlExtractor)
    , _sessionId(0)

{
    _escapeSequenceUrlExtractor->setScreen(this);
    _lineProperties.resize(_lines + 1);
    std::fill(_lineProperties.begin(), _lineProperties.end(), LINE_DEFAULT);

    initTabStops();
    clearSelection();
    reset();
}

/*! Destructor
*/

Screen::~Screen()
{
    delete _history;
    delete _escapeSequenceUrlExtractor;
}

void Screen::cursorUp(int n)
    //=CUU
{
    if (n < 1) {
        n = 1; // Default
    }
    const int stop = _cuY < _topMargin ? 0 : _topMargin;
    _cuY = qMax(stop, _cuY - n);
}

void Screen::cursorDown(int n)
    //=CUD
{
    if (n < 1) {
       n = 1; // Default
    }
    if (n > MAX_SCREEN_ARGUMENT) {
        n = MAX_SCREEN_ARGUMENT;
    }
    const int stop = _cuY > _bottomMargin ? _lines - 1 : _bottomMargin;
    _cuY = qMin(stop, _cuY + n);
}

void Screen::cursorLeft(int n)
    //=CUB
{
    if (n < 1) {
        n = 1; // Default
    }
    _cuX = qMax(0, _cuX - n);
}

void Screen::cursorRight(int n)
    //=CUF
{
    if (n < 1) {
        n = 1; // Default
    }
    if (n > MAX_SCREEN_ARGUMENT) {
        n = MAX_SCREEN_ARGUMENT;
    }
    _cuX = qMin(getScreenLineColumns(_cuY) - 1, _cuX + n);
}

void Screen::setMargins(int top, int bot)
    //=STBM
{
    if (top == 0) top = 1;      // Default
    if (bot == 0) bot = _lines;  // Default
    top = top - 1;              // Adjust to internal lineno
    bot = bot - 1;              // Adjust to internal lineno
    if ( !( 0 <= top && top < bot && bot < _lines ) )
    { //Debug()<<" setRegion("<<top<<","<<bot<<") : bad range.";
        return;                   // Default error action: ignore
    }
    _topMargin = top;
    _bottomMargin = bot;
    _cuX = 0;
    _cuY = getMode(MODE_Origin) ? top : 0;

}

int Screen::topMargin() const
{
    return _topMargin;
}
int Screen::bottomMargin() const
{
    return _bottomMargin;
}

void Screen::index()
    //=IND
{
    if (_cuY == _bottomMargin)
        scrollUp(1);
    else if (_cuY < _lines - 1)
        _cuY += 1;
}

void Screen::reverseIndex()
    //=RI
{
    if (_cuY == _topMargin)
        scrollDown(_topMargin,1);
    else if (_cuY > 0)
        _cuY -= 1;
}

void Screen::nextLine()
    //=NEL
{
    toStartOfLine(); index();
}

void Screen::eraseChars(int n)
{
    if (n < 1) {
        n = 1; // Default
    }
    if (n > MAX_SCREEN_ARGUMENT) {
        n = MAX_SCREEN_ARGUMENT;
    }
    const int p = qBound(0, _cuX + n - 1, _columns - 1);
    clearImage(loc(_cuX, _cuY), loc(p, _cuY), ' ', false);

}

void Screen::deleteChars(int n)
{
    Q_ASSERT( n >= 0 );

    // always delete at least one char
    if (n == 0)
        n = 1;

    // if cursor is beyond the end of the line there is nothing to do
    if ( _cuX >= _screenLines[_cuY].count() )
        return;

    if ( _cuX+n > _screenLines[_cuY].count() )
        n = _screenLines[_cuY].count() - _cuX;

    Q_ASSERT( n >= 0 );
    Q_ASSERT( _cuX+n <= _screenLines[_cuY].count() );

    _screenLines[_cuY].remove(_cuX,n);
}

void Screen::insertChars(int n)
{
    if (n == 0) n = 1; // Default

    if ( _screenLines[_cuY].size() < _cuX )
        _screenLines[_cuY].resize(_cuX);

    _screenLines[_cuY].insert(_cuX,n,' ');

    if ( _screenLines[_cuY].count() > _columns )
        _screenLines[_cuY].resize(_columns);
}

void Screen::repeatChars(int count)
    //=REP
{
    if (count == 0)
    {
        count = 1;
    }
    /**
     * From ECMA-48 version 5, section 8.3.103
     * If the character preceding REP is a control function or part of a
     * control function, the effect of REP is not defined by this Standard.
     *
     * So, a "normal" program should always use REP immediately after a visible
     * character (those other than escape sequences). So, _lastDrawnChar can be
     * safely used.
     */
    for (int i = 0; i < count; i++)
    {
        displayCharacter(_lastDrawnChar);
    }
}

void Screen::deleteLines(int n)
{
    if (n == 0) n = 1; // Default
    scrollUp(_cuY,n);
}

void Screen::insertLines(int n)
{
    if (n == 0) n = 1; // Default
    scrollDown(_cuY,n);
}

void Screen::setMode(int m)
{
    _currentModes[m] = true;
    switch(m)
    {
        case MODE_Origin : _cuX = 0; _cuY = _topMargin; break; //FIXME: home
    }
}

void Screen::resetMode(int m)
{
    _currentModes[m] = false;
    switch(m)
    {
        case MODE_Origin : _cuX = 0; _cuY = 0; break; //FIXME: home
    }
}

void Screen::saveMode(int m)
{
    _savedModes[m] = _currentModes[m];
}

void Screen::restoreMode(int m)
{
    _currentModes[m] = _savedModes[m];
}

bool Screen::getMode(int m) const
{
    return _currentModes[m];
}

void Screen::saveCursor()
{
    _savedState.cursorColumn = _cuX;
    _savedState.cursorLine  = _cuY;
    _savedState.rendition = _currentRendition;
    _savedState.foreground = _currentForeground;
    _savedState.background = _currentBackground;
}

void Screen::restoreCursor()
{
    _cuX     = qMin(_savedState.cursorColumn, _columns-1);
    _cuY     = qMin(_savedState.cursorLine,_lines - 1);
    _currentRendition    = _savedState.rendition;
    _currentForeground   = _savedState.foreground;
    _currentBackground   = _savedState.background;
    updateEffectiveRendition();
}

void Screen::resizeImage(int new_lines, int new_columns)
{
    if ((new_lines == _lines) && (new_columns == _columns)) {
        return;
    }

    // Adjust scroll position, and fix glitches
    _oldTotalLines = getLines() + getHistLines();
    _isResize = true;

    int cursorLine = getCursorLine();
    const int oldCursorLine = (cursorLine == _lines - 1 || cursorLine > new_lines - 1) ? new_lines - 1 : cursorLine;

    // Check if _history need to change
    if (_enableReflowLines && new_columns != _columns && _history->getLines() && _history->getMaxLines()) {
        // Join next line from _screenLine to _history
        while (_history->isWrappedLine(_history->getLines() - 1)) {
            fastAddHistLine();
            cursorLine--;
        }
        auto removedLines = _history->reflowLines(new_columns);

        // If _history size > max _history size it will drop a line from _history.
        // We need to verify if we need to remove a URL.
        if (removedLines) {
            _escapeSequenceUrlExtractor->historyLinesRemoved(removedLines);
        }
    }

    if (_enableReflowLines && new_columns != _columns) {
        int cursorLineCorrection = 0;
        if (_currentTerminalDisplay) {
            // The 'zsh' works different from other shells when writting the command line.
            // It needs to identify the 'zsh' and calculate the new command line.
            auto currSession = _sessionId > 0 ? SessionManager::instance()->idToSession(_sessionId) : nullptr;
            if (currSession
                    && (QLatin1String("zsh") == currSession->foregroundProcessName())
                    && (cursorLine > 0)
                    && ((_lineProperties[cursorLine - 1] & LINE_WRAPPED) != 0)) {
                while (cursorLine + cursorLineCorrection > 0 && (_lineProperties[cursorLine + cursorLineCorrection - 1] & LINE_WRAPPED) != 0) {
                    cursorLineCorrection--;
                }
            }
        }

        // Analize the lines and move the data to lines below.
        int currentPos = 0;
        while (currentPos < (cursorLine + cursorLineCorrection) && currentPos < _screenLines.count() - 1) {
            // Join wrapped line in current position
            if ((_lineProperties[currentPos] & LINE_WRAPPED) != 0) {
                _screenLines[currentPos].append(std::move(_screenLines[currentPos + 1]));
                _screenLines.remove(currentPos + 1);
                _lineProperties.remove(currentPos);
                cursorLine--;
                continue;
            }

            // Ignore whitespaces at the end of the line
            int lineSize = _screenLines[currentPos].size();
            while (lineSize > 0 && QChar(_screenLines[currentPos][lineSize - 1].character).isSpace()) {
                lineSize--;
            }

            // If need to move to line below, copy from the current line, to the next one.
            if (lineSize > new_columns) {
                auto values = _screenLines[currentPos].mid(new_columns);
                _screenLines[currentPos].resize(new_columns);
                _lineProperties.insert(currentPos + 1, _lineProperties[currentPos]);
                _screenLines.insert(currentPos + 1, std::move(values));
                _lineProperties[currentPos] |= LINE_WRAPPED;
                cursorLine++;
            }
            currentPos += 1;
        }
    }

    // Check if it need to move from _screenLine to _history
    while (cursorLine > new_lines - 1) {
        fastAddHistLine();
        cursorLine--;
    }

    if (_enableReflowLines) {
        // Check cursor position and send from _history to _screenLines
        ImageLine histLine;
        histLine.reserve(1024);
        while (cursorLine < oldCursorLine && _history->getLines()) {
            int histPos = _history->getLines() - 1;
            int histLineLen = _history->getLineLen(histPos);
            LineProperty lineProperty = _history->getLineProperty(histPos);
            histLine.resize(histLineLen);
            _history->getCells(histPos, 0, histLineLen, histLine.data());
            _screenLines.insert(0, histLine);
            _lineProperties.insert(0, lineProperty);
            _history->removeCells();
            cursorLine++;
        }
    }

    _lineProperties.resize(new_lines + 1);
    if (_lineProperties.size() > _screenLines.size()) {
        std::fill(_lineProperties.begin() + _screenLines.size(), _lineProperties.end(), LINE_DEFAULT);
    }
    _screenLines.resize(new_lines + 1);

    _screenLinesSize = new_lines;
    _lines = new_lines;
    _columns = new_columns;
    _cuX = qMin(_cuX, _columns - 1);
    cursorLine = qBound(0, cursorLine, _lines - 1);
    setCursorLine(cursorLine);

    // FIXME: try to keep values, evtl.
    setDefaultMargins();
    initTabStops();
    clearSelection();
}

void Screen::setDefaultMargins()
{
    _topMargin = 0;
    _bottomMargin = _lines - 1;
}


/*
   Clarifying rendition here and in the display.

   currently, the display's color table is
   0       1       2 .. 9    10 .. 17
   dft_fg, dft_bg, dim 0..7, intensive 0..7

   _currentForeground, _currentBackground contain values 0..8;
   - 0    = default color
   - 1..8 = ansi specified color

   re_fg, re_bg contain values 0..17
   due to the TerminalDisplay's color table

   rendition attributes are

   attr           widget screen
   -------------- ------ ------
   RE_UNDERLINE     XX     XX    affects foreground only
   RE_BLINK         XX     XX    affects foreground only
   RE_BOLD          XX     XX    affects foreground only
   RE_REVERSE       --     XX
   RE_TRANSPARENT   XX     --    affects background only
   RE_INTENSIVE     XX     --    affects foreground only

   Note that RE_BOLD is used in both widget
   and screen rendition. Since xterm/vt102
   is to poor to distinguish between bold
   (which is a font attribute) and intensive
   (which is a color attribute), we translate
   this and RE_BOLD in falls eventually appart
   into RE_BOLD and RE_INTENSIVE.
   */

void Screen::reverseRendition(Character& p) const
{
    CharacterColor f = p.foregroundColor;
    CharacterColor b = p.backgroundColor;

    p.foregroundColor = b;
    p.backgroundColor = f; //p->r &= ~RE_TRANSPARENT;
}

void Screen::updateEffectiveRendition()
{
    _effectiveRendition = _currentRendition;
    if (_currentRendition & RE_REVERSE)
    {
        _effectiveForeground = _currentBackground;
        _effectiveBackground = _currentForeground;
    }
    else
    {
        _effectiveForeground = _currentForeground;
        _effectiveBackground = _currentBackground;
    }

    if (_currentRendition & RE_BOLD)
        _effectiveForeground.setIntensive();
}

void Screen::copyFromHistory(Character* dest, int startLine, int count) const
{
    Q_ASSERT( startLine >= 0 && count > 0 && startLine + count <= _history->getLines() );

    for (int line = startLine; line < startLine + count; line++)
    {
        const int length = qMin(_columns, _history->getLineLen(line));
        const int destLineOffset  = (line-startLine) * _columns;

        _history->getCells(line,0,length,dest + destLineOffset);

        for (int column = length; column < _columns; column++)
            dest[destLineOffset+column] = DefaultChar;

        // invert selected text
        if (_selBegin !=-1)
        {
            for (int column = 0; column < _columns; column++)
            {
                if (isSelected(column,line))
                {
                    reverseRendition(dest[destLineOffset + column]);
                }
            }
        }
    }
}

void Screen::copyFromScreen(Character* dest , int startLine , int count) const
{
    Q_ASSERT( startLine >= 0 && count > 0 && startLine + count <= _lines );

    for (int line = startLine; line < (startLine+count) ; line++)
    {
        int srcLineStartIndex  = line * _columns;
        int destLineStartIndex = (line - startLine) * _columns;

        for (int column = 0; column < _columns; column++)
        {
            int srcIndex = srcLineStartIndex + column;
            int destIndex = destLineStartIndex + column;

            dest[destIndex] = _screenLines[srcIndex / _columns].value(srcIndex % _columns, DefaultChar);

            // invert selected text
            if (_selBegin != -1 && isSelected(column,line + _history->getLines()))
                reverseRendition(dest[destIndex]);
        }

    }
}

void Screen::getImage( Character* dest, int size, int startLine, int endLine ) const
{
    Q_ASSERT( startLine >= 0 );
    Q_ASSERT( endLine >= startLine && endLine < _history->getLines() + _lines );

    const int mergedLines = endLine - startLine + 1;

    Q_ASSERT( size >= mergedLines * _columns );
    Q_UNUSED( size );

    const int linesInHistoryBuffer = qBound(0, _history->getLines()-startLine,mergedLines);
    const int linesInScreenBuffer = mergedLines - linesInHistoryBuffer;

    // copy _lines from _history buffer
    if (linesInHistoryBuffer > 0)
        copyFromHistory(dest,startLine,linesInHistoryBuffer);

    // copy _lines from screen buffer
    if (linesInScreenBuffer > 0)
        copyFromScreen(dest + linesInHistoryBuffer * _columns,
                startLine + linesInHistoryBuffer - _history->getLines(),
                linesInScreenBuffer);

    // invert display when in screen mode
    if (getMode(MODE_Screen))
    {
        for (int i = 0; i < mergedLines * _columns; i++)
            reverseRendition(dest[i]); // for reverse display
    }

    // mark the character at the current cursor position
    int cursorIndex = loc(_cuX, _cuY + linesInHistoryBuffer);
    if(getMode(MODE_Cursor) && cursorIndex < _columns * mergedLines)
        dest[cursorIndex].rendition |= RE_CURSOR;
}

QVector<LineProperty> Screen::getLineProperties( int startLine , int endLine ) const
{
    Q_ASSERT( startLine >= 0 );
    Q_ASSERT( endLine >= startLine && endLine < _history->getLines() + _lines );

    const int mergedLines = endLine-startLine+1;
    const int linesInHistory = qBound(0, _history->getLines()-startLine,mergedLines);
    const int linesInScreen = mergedLines - linesInHistory;

    QVector<LineProperty> result(mergedLines);
    int index = 0;

    // copy properties for _lines in _history
    for (int line = startLine; line < startLine + linesInHistory; line++)
    {
        result[index] = _history->getLineProperty(line);
        index++;
    }

    // copy properties for lines in screen buffer
    const int firstScreenLine = startLine + linesInHistory - _history->getLines();
    for (int line = firstScreenLine; line < firstScreenLine+linesInScreen; line++)
    {
        result[index] = _lineProperties[line];
        index++;
    }

    return result;
}

int Screen::getScreenLineColumns(const int line) const
{
    const int doubleWidthLine = _lineProperties[line] & LINE_DOUBLEWIDTH;

    if (doubleWidthLine) {
        return _columns / 2;
    }

    return _columns;
}

void Screen::reset(bool clearScreen)
{
    setMode(MODE_Wrap  ); saveMode(MODE_Wrap  );  // wrap at end of margin
    resetMode(MODE_Origin); saveMode(MODE_Origin);  // position refere to [1,1]
    resetMode(MODE_Insert); saveMode(MODE_Insert);  // overstroke
    setMode(MODE_Cursor);                         // cursor visible
    resetMode(MODE_Screen);                         // screen not inverse
    resetMode(MODE_NewLine);

    _topMargin=0;
    _bottomMargin=_lines - 1;

    setDefaultRendition();
    saveCursor();

    if ( clearScreen )
        clear();
}

void Screen::clear()
{
    clearEntireScreen();
    home();
}

void Screen::backspace()
{
    _cuX = qMin(_columns - 1, _cuX); // nowrap!
    _cuX = qMax(0, _cuX - 1);

    if (_screenLines[_cuY].size() < _cuX + 1)
        _screenLines[_cuY].resize(_cuX + 1);

    if (BS_CLEARS)
        _screenLines[_cuY][_cuX].character = ' ';
}

void Screen::tab(int n)
{
    // note that TAB is a format effector (does not write ' ');
    if (n == 0) n = 1;
    while((n > 0) && (_cuX < _columns-1))
    {
        cursorRight(1);
        while((_cuX < _columns - 1) && !_tabStops[_cuX]) {
            int oldXValue = _cuX;
            cursorRight(1);
            if (oldXValue == _cuX) {
                break;
            }
        }
        n--;
    }
}

void Screen::backtab(int n)
{
    // note that TAB is a format effector (does not write ' ');
    if (n == 0) n = 1;
    while((n > 0) && (_cuX > 0))
    {
        cursorLeft(1); while((_cuX > 0) && !_tabStops[_cuX]) cursorLeft(1);
        n--;
    }
}

void Screen::clearTabStops()
{
    for (int i = 0; i < _columns; i++) _tabStops[i] = false;
}

void Screen::changeTabStop(bool set)
{
    if (_cuX >= _columns) return;
    _tabStops[_cuX] = set;
}

void Screen::initTabStops()
{
    _tabStops.resize(_columns);

    // Arrg! The 1st tabstop has to be one longer than the other.
    // i.e. the kids start counting from 0 instead of 1.
    // Other programs might behave correctly. Be aware.
    for (int i = 0; i < _columns; i++)
        _tabStops[i] = (i%8 == 0 && i != 0);
}

void Screen::newLine()
{
    if (getMode(MODE_NewLine))
        toStartOfLine();
    index();
}

void Screen::checkSelection(int from, int to)
{
    if (_selBegin == -1)
        return;
    int scr_TL = loc(0, _history->getLines());
    //Clear entire selection if it overlaps region [from, to]
    if ( (_selBottomRight >= (from+scr_TL)) && (_selTopLeft <= (to+scr_TL)) )
        clearSelection();
}

void Screen::displayCharacter(uint c)
{
    // Note that VT100 does wrapping BEFORE putting the character.
    // This has impact on the assumption of valid cursor positions.
    // We indicate the fact that a newline has to be triggered by
    // putting the cursor one right to the last column of the screen.

    int w = Character::width(c);
    if (w <= 0)
        return;

    if (_cuX + w > _columns) {
        if (getMode(MODE_Wrap)) {
            _lineProperties[_cuY] = (LineProperty)(_lineProperties[_cuY] | LINE_WRAPPED);
            nextLine();
        }
        else
            _cuX = _columns - w;
    }

    // ensure current line vector has enough elements
    if (_cuY >= _screenLines.size())
        return;
    int size = _screenLines[_cuY].size();
    if (size < _cuX+w)
    {
        _screenLines[_cuY].resize(_cuX+w);
    }

    if (getMode(MODE_Insert)) insertChars(w);

    _lastPos = loc(_cuX,_cuY);

    // check if selection is still valid.
    checkSelection(_lastPos, _lastPos);

    Character& currentChar = _screenLines[_cuY][_cuX];

    currentChar.character = c;
    currentChar.foregroundColor = _effectiveForeground;
    currentChar.backgroundColor = _effectiveBackground;
    currentChar.rendition = _effectiveRendition;

    _lastDrawnChar = c;

    int i = 0;
    int newCursorX = _cuX + w--;
    while(w)
    {
        i++;

        if ( _screenLines[_cuY].size() < _cuX + i + 1 )
            _screenLines[_cuY].resize(_cuX+i+1);

        Character& ch = _screenLines[_cuY][_cuX + i];
        ch.character = 0;
        ch.foregroundColor = _effectiveForeground;
        ch.backgroundColor = _effectiveBackground;
        ch.rendition = _effectiveRendition;

        w--;
    }
    _cuX = newCursorX;
    _escapeSequenceUrlExtractor->appendUrlText(QChar(c));
}

void Screen::compose(const QString& /*compose*/)
{
    Q_ASSERT( 0 /*Not implemented yet*/ );

    /*  if (_lastPos == -1)
        return;

        QChar c(image[_lastPos].character);
        compose.prepend(c);
    //compose.compose(); ### FIXME!
    image[_lastPos].character = compose[0].unicode();*/
}

int Screen::scrolledLines() const
{
    return _scrolledLines;
}
int Screen::droppedLines() const
{
    return _droppedLines;
}
void Screen::resetDroppedLines()
{
    _droppedLines = 0;
}
void Screen::resetScrolledLines()
{
    _scrolledLines = 0;
}

void Screen::scrollUp(int n)
{
    if (n < 1) {
        n = 1; // Default
    }
    if (_topMargin == 0) {
        addHistLine(); // history.history
    }
    scrollUp(_topMargin, n);
}

QRect Screen::lastScrolledRegion() const
{
    return _lastScrolledRegion;
}

void Screen::scrollUp(int from, int n)
{
    if (n <= 0)
        return;
    if (from > _bottomMargin)
        return;
    if (from + n > _bottomMargin)
        n = _bottomMargin + 1 - from;

    _scrolledLines -= n;
    _lastScrolledRegion = QRect(0, _topMargin, _columns - 1,(_bottomMargin - _topMargin));

    //FIXME: make sure `topMargin', `bottomMargin', `from', `n' is in bounds.
    moveImage(loc(0, from), loc(0,from + n), loc(_columns, _bottomMargin));
    clearImage(loc(0, _bottomMargin - n + 1), loc(_columns - 1, _bottomMargin), ' ');
}

void Screen::scrollDown(int n)
{
    if (n == 0) n = 1; // Default
    scrollDown(_topMargin, n);
}

void Screen::scrollDown(int from, int n)
{
    _scrolledLines += n;

    //FIXME: make sure `topMargin', `bottomMargin', `from', `n' is in bounds.
    if (n <= 0)
        return;
    if (from > _bottomMargin)
        return;
    if (from + n > _bottomMargin)
        n = _bottomMargin - from;
    moveImage(loc(0,from+n),loc(0,from),loc(_columns - 1,_bottomMargin - n));
    clearImage(loc(0,from),loc(_columns - 1,from+n-1),' ');
}

void Screen::setCursorYX(int y, int x)
{
    setCursorY(y); setCursorX(x);
}

void Screen::setCursorX(int x)
{
    if (x == 0) x = 1; // Default
    x -= 1; // Adjust
    _cuX = qMax(0,qMin(_columns - 1, x));
}

void Screen::setCursorY(int y)
{
    if (y < 1) {
        y = 1; // Default
    }
    if (y > MAX_SCREEN_ARGUMENT) {
        y = MAX_SCREEN_ARGUMENT;
    }
    y += (getMode(MODE_Origin) ? _topMargin : 0);
    _cuY = qBound(0, y - 1, _lines - 1);
}

void Screen::home()
{
    _cuX = 0;
    _cuY = 0;
}

void Screen::toStartOfLine()
{
    _cuX = 0;
}

int Screen::getCursorX() const
{
    return _cuX;
}

int Screen::getCursorY() const
{
    return _cuY;
}

void Screen::clearImage(int loca, int loce, char c, bool resetLineRendition)
{
    int scr_TL=loc(0, _history->getLines());
    //FIXME: check positions

    //Clear entire selection if it overlaps region to be moved...
    if ( (_selBottomRight > (loca+scr_TL) )&&(_selTopLeft < (loce+scr_TL)) )
    {
        clearSelection();
    }

    int topLine = loca / _columns;
    int bottomLine = loce / _columns;

    Character clearCh(c, _currentForeground, _currentBackground, DEFAULT_RENDITION);

    //if the character being used to clear the area is the same as the
    //default character, the affected _lines can simply be shrunk.
    bool isDefaultCh = (clearCh == Character());

    for (int y=topLine;y<=bottomLine;y++)
    {
        if(y >= _lineProperties.size())
            break;
        _lineProperties[y] = 0;

        int endCol = ( y == bottomLine) ? loce % _columns : _columns - 1;
        int startCol = ( y == topLine ) ? loca % _columns : 0;

        QVector<Character>& line = _screenLines[y];

        if ( isDefaultCh && endCol == _columns - 1 )
        {
            line.resize(startCol);
        }
        else
        {
            if (line.size() < endCol + 1)
                line.resize(endCol+1);

            Character* data = line.data();
            for (int i=startCol;i<=endCol;i++)
                data[i]=clearCh;
        }

        if (resetLineRendition && startCol == 0 && endCol == _columns - 1) {
                _lineProperties[y] &= ~(LINE_DOUBLEWIDTH | LINE_DOUBLEHEIGHT_TOP | LINE_DOUBLEHEIGHT_BOTTOM);
        }
    }
}

void Screen::moveImage(int dest, int sourceBegin, int sourceEnd)
{
    Q_ASSERT( sourceBegin <= sourceEnd );

    int lines = (sourceEnd - sourceBegin) / _columns;

    //move screen image and line properties:
    //the source and destination areas of the image may overlap,
    //so it matters that we do the copy in the right order -
    //forwards if dest < sourceBegin or backwards otherwise.
    //(search the web for 'memmove implementation' for details)
    if (dest < sourceBegin)
    {
        for (int i=0;i<=lines;i++)
        {
            _screenLines[(dest / _columns)+i ] = _screenLines[ (sourceBegin / _columns)+i ];
            _lineProperties[(dest / _columns)+i] =_lineProperties[(sourceBegin / _columns)+i];
        }
    }
    else
    {
        for (int i=lines;i>=0;i--)
        {
            _screenLines[(dest / _columns)+i ] = _screenLines[ (sourceBegin / _columns)+i ];
            _lineProperties[(dest / _columns)+i] =_lineProperties[(sourceBegin / _columns)+i];
        }
    }

    if (_lastPos != -1)
    {
        int diff = dest - sourceBegin; // Scroll by this amount
        _lastPos += diff;
        if ((_lastPos < 0) || (_lastPos >= (lines * _columns)))
            _lastPos = -1;
    }

    // Adjust selection to follow scroll.
    if (_selBegin != -1)
    {
        bool beginIsTL = (_selBegin == _selTopLeft);
        int diff = dest - sourceBegin; // Scroll by this amount
        int scr_TL=loc(0, _history->getLines());
        int srca = sourceBegin+scr_TL; // Translate index from screen to global
        int srce = sourceEnd+scr_TL; // Translate index from screen to global
        int desta = srca+diff;
        int deste = srce+diff;

        if ((_selTopLeft >= srca) && (_selTopLeft <= srce))
            _selTopLeft += diff;
        else if ((_selTopLeft >= desta) && (_selTopLeft <= deste))
            _selBottomRight = -1; // Clear selection (see below)

        if ((_selBottomRight >= srca) && (_selBottomRight <= srce))
            _selBottomRight += diff;
        else if ((_selBottomRight >= desta) && (_selBottomRight <= deste))
            _selBottomRight = -1; // Clear selection (see below)

        if (_selBottomRight < 0)
        {
            clearSelection();
        }
        else
        {
            if (_selTopLeft < 0)
                _selTopLeft = 0;
        }

        if (beginIsTL)
            _selBegin = _selTopLeft;
        else
            _selBegin = _selBottomRight;
    }
}

void Screen::clearToEndOfScreen()
{
    clearImage(loc(_cuX,_cuY), loc(_columns - 1, _lines - 1), ' ');
}

void Screen::clearToBeginOfScreen()
{
    clearImage(loc(0, 0), loc(_cuX, _cuY), ' ');
}

void Screen::clearEntireScreen()
{
    clearImage(loc(0, 0), loc(_columns - 1, _lines - 1), ' ');
}

/*! fill screen with 'E'
  This is to aid screen alignment
  */

void Screen::helpAlign()
{
    clearImage(loc(0, 0), loc(_columns - 1, _lines - 1), 'E');
}

void Screen::clearToEndOfLine()
{
    clearImage(loc(_cuX,_cuY),loc(_columns - 1,_cuY),' ');
}

void Screen::clearToBeginOfLine()
{
    clearImage(loc(0,_cuY),loc(_cuX,_cuY),' ');
}

void Screen::clearEntireLine()
{
    clearImage(loc(0, _cuY), loc(_columns - 1, _cuY), ' ');
}

void Screen::setRendition(int re)
{
    _currentRendition |= re;
    updateEffectiveRendition();
}

void Screen::resetRendition(int re)
{
    _currentRendition &= ~re;
    updateEffectiveRendition();
}

void Screen::setDefaultRendition()
{
    setForeColor(COLOR_SPACE_DEFAULT,DEFAULT_FORE_COLOR);
    setBackColor(COLOR_SPACE_DEFAULT,DEFAULT_BACK_COLOR);
    _currentRendition   = DEFAULT_RENDITION;
    updateEffectiveRendition();
}

void Screen::setForeColor(int space, int color)
{
    _currentForeground = CharacterColor(space, color);

    if ( _currentForeground.isValid() )
        updateEffectiveRendition();
    else
        setForeColor(COLOR_SPACE_DEFAULT,DEFAULT_FORE_COLOR);
}

void Screen::setBackColor(int space, int color)
{
    _currentBackground = CharacterColor(space, color);

    if ( _currentBackground.isValid() )
        updateEffectiveRendition();
    else
        setBackColor(COLOR_SPACE_DEFAULT,DEFAULT_BACK_COLOR);
}

void Screen::clearSelection()
{
    _selBottomRight = -1;
    _selTopLeft = -1;
    _selBegin = -1;
}

void Screen::getSelectionStart(int& column , int& line) const
{
    if ( _selTopLeft != -1 )
    {
        column = _selTopLeft % _columns;
        line = _selTopLeft / _columns;
    }
    else
    {
        column = _cuX + getHistLines();
        line = _cuY + getHistLines();
    }
}
void Screen::getSelectionEnd(int& column , int& line) const
{
    if ( _selBottomRight != -1 )
    {
        column = _selBottomRight % _columns;
        line = _selBottomRight / _columns;
    }
    else
    {
        column = _cuX + getHistLines();
        line = _cuY + getHistLines();
    }
}
void Screen::setSelectionStart(const int x, const int y, const bool mode)
{
    _selBegin = loc(x,y);
    /* FIXME, HACK to correct for x too far to the right... */
    if (x == _columns) _selBegin--;

    _selBottomRight = _selBegin;
    _selTopLeft = _selBegin;
    _blockSelectionMode = mode;
}

void Screen::setSelectionEnd( const int x, const int y)
{
    if (_selBegin == -1)
        return;

    int endPos =  loc(x,y);

    if (endPos < _selBegin)
    {
        _selTopLeft = endPos;
        _selBottomRight = _selBegin;
    }
    else
    {
        /* FIXME, HACK to correct for x too far to the right... */
        if (x == _columns)
            endPos--;

        _selTopLeft = _selBegin;
        _selBottomRight = endPos;
    }

    // Normalize the selection in column mode
    if (_blockSelectionMode)
    {
        int topRow = _selTopLeft / _columns;
        int topColumn = _selTopLeft % _columns;
        int bottomRow = _selBottomRight / _columns;
        int bottomColumn = _selBottomRight % _columns;

        _selTopLeft = loc(qMin(topColumn, bottomColumn), topRow);
        _selBottomRight = loc(qMax(topColumn, bottomColumn), bottomRow);
    }
}
/********************************************************************
 1. @函数:    setSelectionAll
 2. @作者:     王培利
 3. @日期:     2020-02-13
 4. @说明:    屏幕全选功能
********************************************************************/
void Screen::setSelectionAll()
{
    _selBegin   = 0;
    _selTopLeft  = 0;
    int endPos = (getHistLines() + getCursorY() + 1) * _columns - 1;
    _selBottomRight = endPos;
}

bool Screen::isSelected(const int x, const int y) const
{
    bool columnInSelection = true;
    if (_blockSelectionMode)
    {
        columnInSelection = x >= (_selTopLeft % _columns) &&
            x <= (_selBottomRight % _columns);
    }

    int pos = loc(x,y);
    return pos >= _selTopLeft && pos <= _selBottomRight && columnInSelection;
}

QString Screen::selectedText(const DecodingOptions options) const
{
    if (!isSelectionValid()) {
        return QString();
    }

    return text(_selTopLeft, _selBottomRight, options);
}

QString Screen::text(int startIndex, int endIndex, const DecodingOptions options) const
{
    QString result;
    QTextStream stream(&result, QIODevice::ReadWrite);

    HTMLDecoder htmlDecoder;
    PlainTextDecoder plainTextDecoder;

    TerminalCharacterDecoder *decoder;
    if((options & ConvertToHtml) != 0U) {
        decoder = &htmlDecoder;
    } else {
        decoder = &plainTextDecoder;
    }

    decoder->begin(&stream);
    writeToStream(decoder, startIndex, endIndex, options);
    decoder->end();

    return result;
}

bool Screen::isSelectionValid() const
{
    return _selTopLeft >= 0 && _selBottomRight >= 0;
}

void Screen::writeSelectionToStream(TerminalCharacterDecoder* decoder, const DecodingOptions options) const
{
    if (!isSelectionValid())
        return;
    writeToStream(decoder,_selTopLeft,_selBottomRight,options);
}

void Screen::writeToStream(TerminalCharacterDecoder* decoder,
                           int startIndex,
                           int endIndex,
                           const DecodingOptions options) const
{
    int top = startIndex / _columns;
    int left = startIndex % _columns;

    int bottom = endIndex / _columns;
    int right = endIndex % _columns;

    Q_ASSERT( top >= 0 && left >= 0 && bottom >= 0 && right >= 0 );

    for (int y=top;y<=bottom;y++)
    {
        int start = 0;
        if ( y == top || _blockSelectionMode ) start = left;

        int count = -1;
        if ( y == bottom || _blockSelectionMode ) count = right - start + 1;

        const bool appendNewLine = ( y != bottom );
        int copied = copyLineToStream( y,
                start,
                count,
                decoder,
                appendNewLine,
                options );

        // if the selection goes beyond the end of the last line then
        // append a new line character.
        //
        // this makes it possible to 'select' a trailing new line character after
        // the text on a line.
        if ( y == bottom &&
                copied < count    )
        {
            Character newLineChar('\n');
            decoder->decodeLine(&newLineChar,1,0);
        }
    }
}

int Screen::getLineLength(const int line) const
{
    // determine if the line is in the history buffer or the screen image
    const bool isInHistoryBuffer = line < _history->getLines();

    if (isInHistoryBuffer) {
        return _history->getLineLen(line);
    }

    return _columns;
}

Character *Screen::getCharacterBuffer(const int size)
{
    // buffer to hold characters for decoding
    // the buffer is static to avoid initializing every
    // element on each call to copyLineToStream
    // (which is unnecessary since all elements will be overwritten anyway)
    static const int MAX_CHARS = 1024;
    static QVector<Character> characterBuffer(MAX_CHARS);

    if (characterBuffer.count() < size) {
        characterBuffer.resize(size);
    }

    return characterBuffer.data();
}

int Screen::copyLineToStream(int line ,
        int start,
        int count,
        TerminalCharacterDecoder* decoder,
        bool appendNewLine,
        const DecodingOptions options) const
{
    const int lineLength = getLineLength(line);
    // ensure that this method, can append space or 'eol' character to
    // the selection
    Character *characterBuffer = getCharacterBuffer((count > -1 ? count : lineLength - start) + 1);
    LineProperty currentLineProperties = 0;

    // determine if the line is in the history buffer or the screen image
    if (line < _history->getLines()) {
        // ensure that start position is before end of line
        start = qBound(0, start, lineLength - 1);

        // retrieve line from history buffer.  It is assumed
        // that the history buffer does not store trailing white space
        // at the end of the line, so it does not need to be trimmed here
        if (count == -1) {
            count = lineLength - start;
        } else {
            count = qMin(start + count, lineLength) - start;
        }

        // safety checks
        Q_ASSERT(start >= 0);
        Q_ASSERT(count >= 0);
        Q_ASSERT((start + count) <= _history->getLineLen(line));

        _history->getCells(line, start, count, characterBuffer);

        if (_history->isWrappedLine(line)) {
            currentLineProperties |= LINE_WRAPPED;
        }
    } else {
        if (count == -1) {
            count = lineLength - start;
        }

        Q_ASSERT(count >= 0);

        int screenLine = line - _history->getLines();

        Q_ASSERT(screenLine <= _screenLinesSize);

        screenLine = qMin(screenLine, _screenLinesSize);

        auto* data = _screenLines[screenLine].data();
        int length = _screenLines[screenLine].count();

        // Don't remove end spaces in lines that wrap
        if (options.testFlag(TrimTrailingWhitespace) && ((_lineProperties[screenLine] & LINE_WRAPPED) == 0))
        {
            // ignore trailing white space at the end of the line
            while (length > 0 && QChar(data[length - 1].character).isSpace()) {
                length--;
            }
        }

        //retrieve line from screen image
        auto end = qMin(start + count, length);
        if (start < end) {
            std::copy(data + start, data + end, characterBuffer);
        }

        // count cannot be any greater than length
        count = qBound(0, count, length - start);

        Q_ASSERT(screenLine < _lineProperties.count());
        currentLineProperties |= _lineProperties[screenLine];
    }

    if (appendNewLine) {
        if ((currentLineProperties & LINE_WRAPPED) != 0) {
            // do nothing extra when this line is wrapped.
        } else {
            // When users ask not to preserve the linebreaks, they usually mean:
            // `treat LINEBREAK as SPACE, thus joining multiple _lines into
            // single line in the same way as 'J' does in VIM.`
            characterBuffer[count] = options.testFlag(PreserveLineBreaks) ? Character('\n') : Character(' ');
            count++;
        }
    }

    if ((options & TrimLeadingWhitespace) != 0U) {
        int spacesCount = 0;
        for (spacesCount = 0; spacesCount < count; spacesCount++) {
            if (QChar::category(characterBuffer[spacesCount].character) != QChar::Category::Separator_Space) {
                break;
            }
        }

        if (spacesCount >= count) {
            return 0;
        }

        std::copy(characterBuffer + spacesCount, characterBuffer + count, characterBuffer);

        count -= spacesCount;
    }

    //decode line and write to text stream
    decoder->decodeLine(characterBuffer,
                        count, currentLineProperties);

    return count;
}

void Screen::writeLinesToStream(TerminalCharacterDecoder* decoder, int fromLine, int toLine) const
{
    writeToStream(decoder, loc(0,fromLine), loc(_columns-1,toLine), PreserveLineBreaks);
}

void Screen::fastAddHistLine()
{
    const bool removeLine = _history->getLines() == _history->getMaxLines();
    _history->addCellsVector(_screenLines[0]);
    _history->addLine(_lineProperties[0]);

    // If history size > max history size it will drop a line from history.
    // We need to verify if we need to remove a URL.
    if (removeLine) {
        _escapeSequenceUrlExtractor->historyLinesRemoved(1);
    }

    _screenLines.pop_front();
    _lineProperties.remove(0);
}

void Screen::addHistLine()
{
    // add line to history buffer
    // we have to take care about scrolling, too...

    if (hasScroll())
    {
        int oldHistLines = _history->getLines();

        _history->addCellsVector(_screenLines[0]);
        _history->addLine(static_cast<bool>(_lineProperties[0] & LINE_WRAPPED ));

        int newHistLines = _history->getLines();

        bool beginIsTL = (_selBegin == _selTopLeft);

        // If the history is full, increment the count
        // of dropped _lines
        if ( newHistLines == oldHistLines )
            _droppedLines++;

        // Adjust selection for the new point of reference
        if (newHistLines > oldHistLines)
        {
            if (_selBegin != -1)
            {
                _selTopLeft += _columns;
                _selBottomRight += _columns;
            }
        }

        if (_selBegin != -1)
        {
            // Scroll selection in history up
            int top_BR = loc(0, 1+newHistLines);

            if (_selTopLeft < top_BR)
                _selTopLeft -= _columns;

            if (_selBottomRight < top_BR)
                _selBottomRight -= _columns;

            if (_selBottomRight < 0)
                clearSelection();
            else
            {
                if (_selTopLeft < 0)
                    _selTopLeft = 0;
            }

            if (beginIsTL)
                _selBegin = _selTopLeft;
            else
                _selBegin = _selBottomRight;
        }
    }

}

int Screen::getHistLines() const
{
    return _history->getLines();
}

void Screen::setScroll(const HistoryType& t , bool copyPreviousScroll)
{
    clearSelection();

    if ( copyPreviousScroll )
        _history = t.scroll(_history);
    else
    {
        HistoryScroll* oldScroll = _history;
        _history = t.scroll(nullptr);
        delete oldScroll;
    }
}

bool Screen::hasScroll() const
{
    return _history->hasScroll();
}

const HistoryType& Screen::getScroll() const
{
    return _history->getType();
}

void Screen::setLineProperty(LineProperty property , bool enable)
{
    if ( enable )
        _lineProperties[_cuY] = (LineProperty)(_lineProperties[_cuY] | property);
    else
        _lineProperties[_cuY] = (LineProperty)(_lineProperties[_cuY] & ~property);
}
void Screen::fillWithDefaultChar(Character* dest, int count)
{
    for (int i=0;i<count;i++)
        dest[i] = DefaultChar;
}

int Screen::getOldTotalLines()
{
    return _oldTotalLines;
}

bool Screen::isResize()
{
    if (_isResize) {
        _isResize = false;
        return true;
    }
    return _isResize;
}

void Screen::setReflowLines(bool enable)
{
    _enableReflowLines = enable;
}

void Screen::setSessionId(int sessionId)
{
    _sessionId = sessionId;
}

int Screen::getCursorLine()
{
    if (_currentModes[MODE_AppScreen] == 1) {
        return _savedState.cursorLine;
    }
    return _cuY;
}

void Screen::setCursorLine(int newLine)
{
    if (_currentModes[MODE_AppScreen] == 1) {
        _savedState.cursorLine = newLine;
    } else {
        _cuY = newLine;
    }
}

EscapeSequenceUrlExtractor * Screen::urlExtractor() const
{
    return _escapeSequenceUrlExtractor;
}

