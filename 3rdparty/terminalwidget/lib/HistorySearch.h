/*
    Copyright 2013 Christian Surlykke

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
#ifndef TASK_H
#define	TASK_H

#include <QObject>
#include <QPointer>
#include <QMap>

#include <Session.h>
#include <ScreenWindow.h>

#include "Emulation.h"
#include "TerminalCharacterDecoder.h"

using namespace Konsole;

typedef QPointer<Emulation> EmulationPtr;

class HistorySearch : public QObject
{
    Q_OBJECT

public:
    explicit HistorySearch(EmulationPtr emulation,
                           QString searchText,
                           bool forwards,
                           bool isLastForwards,
                           int startColumn,
                           int startLine,
                           QObject* parent);

    ~HistorySearch() override;

    void search(int currBackwardsPosition, int lastFoundStartColumn, int lastFoundStartLine);

signals:
    void matchFound(int startColumn, int startLine, int endColumn, int endLine, int lastBackwardsPosition, int loseChinese, int matchChinese);
    void noMatchFound();

private:
    bool search(int startColumn, int startLine, int endColumn, int endLine);
    int findLineNumberInString(QList<int> linePositions, int position);


    EmulationPtr m_emulation;
    QString m_searchText;
    bool m_forwards;
    bool m_isLastForwards;
    int m_startColumn;
    int m_startLine;

    int m_foundStartColumn;
    int m_foundStartLine;
    int m_foundEndColumn;
    int m_foundEndLine;

    int m_loseChinese;
    int m_matchChinese;
    int m_currBackwardsPosition;
    int m_lastBackwardsPosition;

    int m_lastFoundStartColumn;
    int m_lastFoundStartLine;
};

#endif	/* TASK_H */

