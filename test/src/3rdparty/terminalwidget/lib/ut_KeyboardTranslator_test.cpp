#include "ut_KeyboardTranslator_test.h"

#include "KeyboardTranslator.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QFileInfo>
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

/*******************************************************************************
 1. @函数:    KeyboardTranslator::Entry类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    setModifiers单元测试
*******************************************************************************/
TEST_F(UT_KeyboardTranslator_Test, setModifiers)
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

    for (int i = 0; i < entry.size(); i++) {
        KeyboardTranslator::Entry keyEntry;
        keyEntry.setText(entry.at(i));
        keyEntry.setModifiers(keyboardModifier.at(i));
        qDebug() << i << " : " << keyEntry.text(true, keyboardModifier.at(i)) << endl;
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

    for (int i = 0; i < entry.size(); i++) {
        KeyboardTranslator::Entry keyEntry;
        keyEntry.setText(entry.at(i));
        keyEntry.setModifiers(keyboardModifier.at(i));
        qDebug() << i << " : " << keyEntry.text(true, keyboardModifier.at(i)) << endl;
        EXPECT_EQ(keyEntry.text(true, keyboardModifier.at(i)), resultList.at(i));
    }
}

/*******************************************************************************
 1. @函数:    KeyboardTranslator类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    loadTranslator单元测试
*******************************************************************************/
TEST_F(UT_KeyboardTranslator_Test, loadTranslator)
{
    KeyboardTranslator *translator = KeyboardTranslatorManager::instance()->loadTranslator(QLatin1String("default"));

    if (!translator) {
        QBuffer textBuffer;
        static const QByteArray defaultTranslatorText;
        textBuffer.setData(defaultTranslatorText);
        textBuffer.open(QIODevice::ReadOnly);
        translator = KeyboardTranslatorManager::instance()->loadTranslator(&textBuffer, QLatin1String("fallback"));
    }
}

/*******************************************************************************
 1. @函数:    KeyboardTranslatorManager类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    findTranslator单元测试
*******************************************************************************/
TEST_F(UT_KeyboardTranslator_Test, findTranslator)
{
    KeyboardTranslatorManager::instance()->findTranslator("default");
}

/*******************************************************************************
 1. @函数:    KeyboardTranslatorManager类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    allTranslators单元测试
*******************************************************************************/
TEST_F(UT_KeyboardTranslator_Test, allTranslators)
{
    KeyboardTranslatorManager::instance()->allTranslators();
}

/*******************************************************************************
 1. @函数:    KeyboardTranslator类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    defaultTranslator单元测试
*******************************************************************************/
TEST_F(UT_KeyboardTranslator_Test, defaultTranslator)
{
    KeyboardTranslatorManager::instance()->defaultTranslator();
}

/*******************************************************************************
 1. @函数:    KeyboardTranslator类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    findEntry单元测试
*******************************************************************************/
TEST_F(UT_KeyboardTranslator_Test, findEntry)
{
    KeyboardTranslator *translator = new KeyboardTranslator(QLatin1String("default"));

    translator->findEntry(Qt::Key_Delete,
                          Qt::NoModifier,
                          KeyboardTranslator::NoState);

    delete translator;
}

/*******************************************************************************
 1. @函数:    KeyboardTranslator类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    addEntry单元测试
*******************************************************************************/
TEST_F(UT_KeyboardTranslator_Test, addEntry)
{
    KeyboardTranslator *translator = new KeyboardTranslator(QLatin1String("default"));

    KeyboardTranslator::Entry entry = translator->findEntry(
                                          Qt::Key_Delete,
                                          Qt::NoModifier,
                                          KeyboardTranslator::NoState);

    KeyboardTranslator::Entry newEntry;
    KeyboardTranslator::States flags = KeyboardTranslator::NoState;
    KeyboardTranslator::States flagMask = KeyboardTranslator::NoState;
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    Qt::KeyboardModifiers modifierMask = Qt::NoModifier;
    KeyboardTranslator::Command command = KeyboardTranslator::NoCommand;
    newEntry.setKeyCode(Qt::Key_Delete);
    newEntry.setState(flags);
    newEntry.setStateMask(flagMask);
    newEntry.setModifiers(modifiers);
    newEntry.setModifierMask(modifierMask);
    newEntry.setText(QByteArray("delete", QString("delete").length()));
    newEntry.setCommand(command);
    translator->addEntry(entry);

    delete translator;
}

/*******************************************************************************
 1. @函数:    KeyboardTranslatorReader类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    writeEntry单元测试
*******************************************************************************/
TEST_F(UT_KeyboardTranslator_Test, writeEntry)
{
    KeyboardTranslator *translator = new KeyboardTranslator(QLatin1String("default"));

    QDir dir(KB_LAYOUT_DIR);
    QStringList filters;
    filters << QLatin1String("*.keytab");

    QStringList list = dir.entryList(filters);
    if (list.size() > 0 && translator->entries().size() > 0) {
        QString path = list.first();

        QFile destination(path);
        bool isOpenSuccess = destination.open(QIODevice::WriteOnly | QIODevice::Text);

        KeyboardTranslatorWriter writer(&destination);

        QListIterator<KeyboardTranslator::Entry> iter(translator->entries());
        while (iter.hasNext()) {
            writer.writeEntry(iter.next());
        }
    }

    delete translator;
}

/*******************************************************************************
 1. @函数:    KeyboardTranslatorReader类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    nextEntry单元测试
*******************************************************************************/
TEST_F(UT_KeyboardTranslator_Test, nextEntry)
{
    QDir dir(KB_LAYOUT_DIR);
    QStringList filters;
    filters << QLatin1String("*.keytab");

    QStringList list = dir.entryList(filters);
    if (list.size() > 0) {
        QString path = list.first();

        QFile source(path);
        source.open(QIODevice::ReadOnly);

        QFileInfo keyTabFileInfo(path);

        KeyboardTranslator *translator = new KeyboardTranslator(keyTabFileInfo.baseName());

        KeyboardTranslatorReader reader(&source);
        while (reader.hasNextEntry()) {
            translator->addEntry(reader.nextEntry());
        }

        source.close();

        delete translator;
    }
}

/*******************************************************************************
 1. @函数:    KeyboardTranslatorReader类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:   createEntry单元测试
*******************************************************************************/
TEST_F(UT_KeyboardTranslator_Test, createEntry)
{
    QDir dir(KB_LAYOUT_DIR);
    QStringList filters;
    filters << QLatin1String("*.keytab");

    QStringList list = dir.entryList(filters);
    if (list.size() > 0) {
        QString path = list.first();

        QFile source(path);
        source.open(QIODevice::ReadOnly);

        QFileInfo keyTabFileInfo(path);

        KeyboardTranslator *translator = new KeyboardTranslator(keyTabFileInfo.baseName());

        //测试KeyboardTranslatorReader类的createEntry
        KeyboardTranslatorReader reader(&source);
        reader.createEntry("", "");

        source.close();
        delete translator;
    }
}

/*******************************************************************************
 1. @函数:    KeyboardTranslatorReader类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    parseAsStateFlag单元测试
*******************************************************************************/
TEST_F(UT_KeyboardTranslator_Test, parseAsStateFlag)
{
    QDir dir(KB_LAYOUT_DIR);
    QStringList filters;
    filters << QLatin1String("*.keytab");

    QStringList list = dir.entryList(filters);
    if (list.size() > 0) {
        QString path = list.first();

        QFile source(path);
        source.open(QIODevice::ReadOnly);

        QFileInfo keyTabFileInfo(path);

        KeyboardTranslator *translator = new KeyboardTranslator(keyTabFileInfo.baseName());

        KeyboardTranslatorReader reader(&source);

        KeyboardTranslator::State state1 = KeyboardTranslator::CursorKeysState;
        KeyboardTranslator::State state2 = KeyboardTranslator::AnsiState;
        KeyboardTranslator::State state3 = KeyboardTranslator::NewLineState;
        KeyboardTranslator::State state4 = KeyboardTranslator::CursorKeysState;
        KeyboardTranslator::State state5 = KeyboardTranslator::AnyModifierState;
        KeyboardTranslator::State state6 = KeyboardTranslator::ApplicationKeypadState;

        //测试KeyboardTranslatorReader类的parseAsStateFlag
        reader.parseAsStateFlag("Ctrl+C", state1);
        reader.parseAsStateFlag("Ctrl+C", state2);
        reader.parseAsStateFlag("Ctrl+C", state3);
        reader.parseAsStateFlag("Ctrl+C", state4);
        reader.parseAsStateFlag("Ctrl+C", state5);
        reader.parseAsStateFlag("Ctrl+C", state6);

        source.close();
        delete translator;
    }
}

/*******************************************************************************
 1. @函数:    KeyboardTranslatorReader类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    parseAsModifier单元测试
*******************************************************************************/
TEST_F(UT_KeyboardTranslator_Test, parseAsModifier)
{
    QDir dir(KB_LAYOUT_DIR);
    QStringList filters;
    filters << QLatin1String("*.keytab");

    QStringList list = dir.entryList(filters);
    if (list.size() > 0) {
        QString path = list.first();

        QFile source(path);
        source.open(QIODevice::ReadOnly);

        QFileInfo keyTabFileInfo(path);

        KeyboardTranslator *translator = new KeyboardTranslator(keyTabFileInfo.baseName());

        KeyboardTranslatorReader reader(&source);
        Qt::KeyboardModifier modifyer1 = Qt::ShiftModifier;
        Qt::KeyboardModifier modifyer2 = Qt::ControlModifier;
        Qt::KeyboardModifier modifyer3 = Qt::AltModifier;
        Qt::KeyboardModifier modifyer4 = Qt::MetaModifier;
        Qt::KeyboardModifier modifyer5 = Qt::KeypadModifier;
        Qt::KeyboardModifier modifyer6 = Qt::NoModifier;

        //测试KeyboardTranslatorReader类的parseAsModifier
        reader.parseAsModifier("Ctrl+A", modifyer1);
        reader.parseAsModifier("Ctrl+B", modifyer2);
        reader.parseAsModifier("Ctrl+C", modifyer3);
        reader.parseAsModifier("Ctrl+D", modifyer4);
        reader.parseAsModifier("Ctrl+E", modifyer5);
        reader.parseAsModifier("Ctrl+F", modifyer6);

        source.close();
        delete translator;
    }
}

/*******************************************************************************
 1. @函数:    KeyboardTranslatorReader类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    decodeSequence单元测试
*******************************************************************************/
TEST_F(UT_KeyboardTranslator_Test, decodeSequence)
{
    QDir dir(KB_LAYOUT_DIR);
    QStringList filters;
    filters << QLatin1String("*.keytab");

    QStringList list = dir.entryList(filters);
    if (list.size() > 0) {
        QString path = list.first();

        QFile source(path);
        source.open(QIODevice::ReadOnly);

        QFileInfo keyTabFileInfo(path);

        KeyboardTranslator *translator = new KeyboardTranslator(keyTabFileInfo.baseName());

        //测试KeyboardTranslatorReader类的decodeSequence
        KeyboardTranslatorReader reader(&source);

        KeyboardTranslator::States flags = KeyboardTranslator::NoState;
        KeyboardTranslator::States flagMask = KeyboardTranslator::NoState;
        Qt::KeyboardModifiers modifiers = Qt::NoModifier;
        Qt::KeyboardModifiers modifierMask = Qt::NoModifier;

        int keyCode = Qt::Key_unknown;

        reader.decodeSequence("abcdefghijklmn",
                              keyCode,
                              modifiers,
                              modifierMask,
                              flags,
                              flagMask);

        source.close();
        delete translator;
    }
}

/*******************************************************************************
 1. @函数:    KeyboardTranslator类的函数
 2. @作者:    ut000438 王亮
 3. @日期:    2020-12-28
 4. @说明:    unescape单元测试
*******************************************************************************/
TEST_F(UT_KeyboardTranslator_Test, unescape)
{
    KeyboardTranslator::Entry keyEntry;

    keyEntry.unescape("\\E\\b\\f\\t\\r\\n\\x");
}

#endif
