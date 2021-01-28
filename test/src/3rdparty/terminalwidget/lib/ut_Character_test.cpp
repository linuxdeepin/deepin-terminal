#include "ut_Character_test.h"

#include "Character.h"
#include "TerminalCharacterDecoder.h"
#include "konsole_wcwidth.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_Character_Test::UT_Character_Test()
{
}

void UT_Character_Test::SetUp()
{
}

void UT_Character_Test::TearDown()
{
}

#ifdef UT_CHARACTER_TEST

class PainterWidget :  public QWidget
{
public:
    PainterWidget(QWidget* parent = nullptr) : QWidget(parent)
    {
        resize(800, 600);
        setWindowTitle(tr("Paint Demo"));
    }
protected:
    void paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event);
        QPainter painter(this);

        int fontWidth = 15;
        int fontHeight = 15;

        QStringList drawStringList;
        QString str = QString("abcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*()");
        QString str2 = QString("╌╍╎╏┄┅┆┇┈┉┊┋╚╝╔ ╗╠ ╩ ╣ ╦╬╨╞ ╥ ╡║ ╫ ═ ╪");
        QString str3 = QString("╄  ╃ ╆ ╅┗  ┛ ┏ ┓└  ┘ ┌ ┐┖  ┙ ┍ ┒┚  ┕ ┎ ┑╘  ╜ ╓ ╕ ╛  ╙ ╒ ╖");
        QString str4 = QString("▖▗▘▙▚▛▜▝▞▞▟");
        drawStringList << str << str2 << str3 << str4;
        QRect cellRect = {0, 0, fontWidth, fontHeight};
        for (int strIndex = 0; strIndex < drawStringList.size(); strIndex++) {
            QString drawStr = drawStringList.at(strIndex);
            for (int i = 0 ; i < drawStr.length(); i++) {
                LineBlockCharacters::draw(painter, cellRect.translated(i * fontWidth, 0), drawStr[i], false);
            }
        }
    }
};

/*******************************************************************************
 1. @函数:    LineBlockCharacters类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    draw单元测试
*******************************************************************************/
TEST_F(UT_Character_Test, draw)
{
    PainterWidget painterWidget;
    painterWidget.resize(800, 600);
    painterWidget.show();

    QPaintEvent *event = new QPaintEvent(painterWidget.rect());
    painterWidget.paintEvent(event);
}

#endif
