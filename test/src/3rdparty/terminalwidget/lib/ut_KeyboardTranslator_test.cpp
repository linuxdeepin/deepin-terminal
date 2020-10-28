#include "ut_KeyboardTranslator_test.h"
#include "KeyboardTranslator.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_KeyboardTranslator_Test::UT_KeyboardTranslator_Test()
{
}

void UT_KeyboardTranslator_Test::SetUp()
{
}

void UT_KeyboardTranslator_Test::TearDown()
{
}

typedef struct {
    QByteArray text;
    QByteArray result;
    bool wildcards;
    Qt::KeyboardModifiers modifiers;
} KeyBoardTranslatorData;

#ifdef UT_KEYBOARDTRANSLATOR_TEST
TEST_F(UT_KeyboardTranslator_Test, KeyboardTranslatorTest)
{
    QList<QByteArray> entry;
    entry << QByteArray("E*")
          << QByteArray("E*")
          << QByteArray("E*")
          << QByteArray("E*");

    QList<QByteArray> resultList;
    resultList << QByteArray("E1") << QByteArray("E2") << QByteArray("E3") << QByteArray("E5");

    QList<Qt::KeyboardModifiers> keyboardModifier;
    keyboardModifier << Qt::NoModifier
                     << Qt::ShiftModifier
                     << Qt::AltModifier
                     << Qt::ControlModifier;

    for(int i=0; i<entry.size(); i++)
    {
        KeyboardTranslator::Entry keyEntry;
        keyEntry.setText(entry.at(i));
        keyEntry.setModifiers(keyboardModifier.at(i));
        qDebug() << i << " : "<< keyEntry.text(true, keyboardModifier.at(i)) << endl;
        EXPECT_EQ(keyEntry.text(true, keyboardModifier.at(i)), resultList.at(i));
    }

    entry.clear();
    entry << QByteArray("\033[24;*~")
          << QByteArray("\033[24;*~")
          << QByteArray("\033[24;*~")
          << QByteArray("\033[24;*~");

    resultList.clear();
    resultList << QByteArray("\033[24;4~") << QByteArray("\033[24;6~") << QByteArray("\033[24;7~") << QByteArray("\033[24;8~");

    keyboardModifier.clear();
    keyboardModifier << (Qt::ShiftModifier | Qt::AltModifier)
                     << (Qt::ShiftModifier | Qt::ControlModifier)
                     << (Qt::ControlModifier | Qt::AltModifier)
                     << (Qt::ShiftModifier | Qt::AltModifier | Qt::ControlModifier);

    for(int i=0; i<entry.size(); i++)
    {
        KeyboardTranslator::Entry keyEntry;
        keyEntry.setText(entry.at(i));
        keyEntry.setModifiers(keyboardModifier.at(i));
        qDebug() << i << " : "<< keyEntry.text(true, keyboardModifier.at(i)) << endl;
        EXPECT_EQ(keyEntry.text(true, keyboardModifier.at(i)), resultList.at(i));
    }
}

#endif
