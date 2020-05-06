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
#include <QApplication>
#include <QTextStream>
#include <QDebug>

#include "TerminalCharacterDecoder.h"
#include "Emulation.h"
#include "HistorySearch.h"

HistorySearch::HistorySearch(EmulationPtr emulation, QRegExp regExp,
                             bool forwards, int startColumn, int startLine,
                             QObject *parent) :
    QObject(parent),
    m_emulation(emulation),
    m_regExp(regExp),
    m_forwards(forwards),
    m_startColumn(startColumn),
    m_startLine(startLine)
{
}

HistorySearch::~HistorySearch()
{
}

void HistorySearch::search()
{
    bool found = false;

    if (! m_regExp.isEmpty()) {
        if (m_forwards) {
            found = search(m_startColumn, m_startLine, -1, m_emulation->lineCount()) || search(0, 0, m_startColumn, m_startLine);
        } else {
            found = search(0, 0, m_startColumn, m_startLine) || search(m_startColumn, m_startLine, -1, m_emulation->lineCount());
        }

        if (found) {
            emit matchFound(m_foundStartColumn, m_foundStartLine, m_foundEndColumn, m_foundEndLine);
        } else {
            emit noMatchFound();
        }
    }

    deleteLater();
}

bool HistorySearch::search(int startColumn, int startLine, int endColumn, int endLine)
{
    qDebug() << "search from" << startColumn << "," << startLine
             <<  "to" << endColumn << "," << endLine;

    int linesRead = 0;
    int linesToRead = endLine - startLine + 1;

    qDebug() << "linesToRead:" << linesToRead;

    // We read process history from (and including) startLine to (and including) endLine in
    // blocks of at most 10K lines so that we do not use unhealthy amounts of memory
    int blockSize;
    while ((blockSize = qMin(10000, linesToRead - linesRead)) > 0) {

        QString string;
        QTextStream searchStream(&string);
        PlainTextDecoder decoder;
        decoder.begin(&searchStream);
        decoder.setRecordLinePositions(true);

        // Calculate lines to read and read them
        int blockStartLine = m_forwards ? startLine + linesRead : endLine - linesRead - blockSize + 1;
        int chunkEndLine = blockStartLine + blockSize - 1;
        m_emulation->writeToStream(&decoder, blockStartLine, chunkEndLine);

        // We search between startColumn in the first line of the string and endColumn in the last
        // line of the string. First we calculate the position (in the string) of endColumn in the
        // last line of the string
        int endPosition;

        // The String that Emulator.writeToStream produces has a newline at the end, and so ends with an
        // empty line - we ignore that.
        int numberOfLinesInString = decoder.linePositions().size() - 1;
        if (numberOfLinesInString > 0 && endColumn > -1) {
            endPosition = decoder.linePositions().at(numberOfLinesInString - 1) + endColumn;
        } else {
            endPosition = string.size();
        }

        // So now we can log for m_regExp in the string between startColumn and endPosition
        QRegExp regEx("[\\x4e00-\\x9fa5]+");
        int lineChinese = 0;

        int matchStart;
        if (m_forwards) {
            /***add begin by ut001121 zhangmeng 20200506 修复BUG22626***/
            //确定行开始及结束位置
            int lineEnd = startColumn;
            int lineStart = string.lastIndexOf('\n', lineEnd);
            //qDebug() << "===lineStart" << lineStart << "lineEnd" << lineEnd ;
            //assert(lineStart <= lineEnd);

            if (lineStart == -1)lineStart = 0;
            else lineStart += 1;

            //确定行字符串及中文字符数
            QString lineStr = string.mid(lineStart, lineEnd);
            lineChinese = lineStr.count(regEx);
            //qDebug() << "lineStr" << lineStr << lineChinese;

            //计算开始位置
            startColumn -= lineChinese;
            /***add end by ut001121***/

            matchStart = string.indexOf(m_regExp, startColumn);
            if (matchStart >= endPosition - lineChinese)
                matchStart = -1;
        } else {

            /***add begin by ut001121 zhangmeng 20200506 修复BUG22626***/
            //qDebug() << string.mid(startColumn, endPosition);

            //确定匹配位置
            matchStart = string.lastIndexOf(m_regExp, endPosition - 1);
            //qDebug() << string.mid(matchStart, endPosition);

            //确定行字符串
            int lineStart = string.lastIndexOf('\n', matchStart) + 1;
            QString lineStr = string.mid(lineStart, matchStart - lineStart);
            //qDebug() << lineStr;

            //计算中文字数
            lineChinese = lineStr.count(regEx);

            //计算结束位置
            endPosition -= lineChinese;
            /***add end by ut001121***/

            matchStart = string.lastIndexOf(m_regExp, endPosition - 1);
            if (matchStart < startColumn)
                matchStart = -1;
        }

        if (matchStart > -1) {

            // 中文字个数统计
            QString txt = m_regExp.pattern();
            QRegExp regEx("[\\x4e00-\\x9fa5]+");
            int ChineseCount = txt.count(regEx);
            qDebug() << txt << "ChineseCount" << ChineseCount;

            // 中文是宽字条，占两列，需要补充处理
            int matchEnd = matchStart + m_regExp.matchedLength() - 1 + ChineseCount;
            qDebug() << "Found in string from" << matchStart << "to" << matchEnd; //在string中的开始和结束位置

            /***mod begin by ut001121 zhangmeng 20200506 修复BUG22626***/
            int lineEnd = matchStart;
            int lineStart = string.lastIndexOf('\n', lineEnd);
            //qDebug() << "lineStart" << lineStart << "lineEnd" << lineEnd ;
            //assert(lineStart <= lineEnd);

            if (lineStart == -1)lineStart = 0;
            else lineStart += 1;

            QString lineStr = string.mid(lineStart, lineEnd - lineStart);
            lineChinese = lineStr.count(regEx);
            //qDebug() << "lineStr" << lineStr << lineChinese;

            // Translate startPos and endPos to startColum, startLine, endColumn and endLine in history.
            int startLineNumberInString = findLineNumberInString(decoder.linePositions(), matchStart);
            m_foundStartColumn = matchStart - decoder.linePositions().at(startLineNumberInString) + lineChinese;
            m_foundStartLine = startLineNumberInString + startLine + linesRead;

            int endLineNumberInString = findLineNumberInString(decoder.linePositions(), matchEnd);
            m_foundEndColumn = matchEnd - decoder.linePositions().at(endLineNumberInString) + lineChinese;
            m_foundEndLine = endLineNumberInString + startLine + linesRead;

            /***mod end by ut001121 zhangmeng***/

            qDebug() << "m_foundStartColumn" << m_foundStartColumn
                     << "m_foundStartLine" << m_foundEndLine
                     << "m_foundEndColumn" << m_foundEndColumn
                     << "m_foundEndLine" << m_foundEndLine;

            return true;
        }


        linesRead += blockSize;
    }

    qDebug() << "Not found";
    return false;
}


int HistorySearch::findLineNumberInString(QList<int> linePositions, int position)
{
    int lineNum = 0;
    while (lineNum + 1 < linePositions.size() && linePositions[lineNum + 1] <= position)
        lineNum++;

    return lineNum;
}
