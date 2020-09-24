#include "ut_Character_test.h"

#include "Character.h"
#include "TerminalCharacterDecoder.h"

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
TEST_F(UT_Character_Test, CharacterTest)
{
}

TEST_F(UT_Character_Test, decodeLine)
{
    Character aLineChar('c');

    QTextStream lineStream(new QString());
    TerminalCharacterDecoder *decoder = new PlainTextDecoder;
    decoder->begin(&lineStream);
    decoder->decodeLine(&aLineChar, 1, 0);
    decoder->end();

    delete decoder;

    Character bLineChar('<');

    QTextStream htmlLineStream(new QString());
    decoder = new HTMLDecoder;
    decoder->begin(&htmlLineStream);
    decoder->decodeLine(&bLineChar, 1, 0);
    decoder->end();

    delete decoder;
}

#endif
