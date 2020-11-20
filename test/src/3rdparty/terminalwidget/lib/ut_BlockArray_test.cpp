#include "ut_BlockArray_test.h"

#include "BlockArray.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>

using namespace Konsole;

UT_BlockArray_Test::UT_BlockArray_Test()
{
}

void UT_BlockArray_Test::SetUp()
{
}

void UT_BlockArray_Test::TearDown()
{
}

#ifdef UT_BLOCKARRAY_TEST

TEST_F(UT_BlockArray_Test, setSize)
{
    BlockArray blockArray;
    bool isSetOk = blockArray.setSize(1000);
    EXPECT_EQ(isSetOk, false);
}

TEST_F(UT_BlockArray_Test, setHistorySize)
{
    BlockArray blockArray;
    bool isSetOk = blockArray.setHistorySize(1000);
    EXPECT_EQ(isSetOk, false);
}

TEST_F(UT_BlockArray_Test, newBlock)
{
    BlockArray blockArray;
    bool isSetOk = blockArray.setSize(1000);
    EXPECT_EQ(isSetOk, false);

    size_t res = blockArray.newBlock();
    EXPECT_EQ(res > 0, true);
}

TEST_F(UT_BlockArray_Test, lastBlock)
{
    BlockArray blockArray;
    bool isSetOk = blockArray.setSize(1000);
    EXPECT_EQ(isSetOk, false);

    size_t res = blockArray.newBlock();
    EXPECT_EQ(res > 0, true);

    Block *lastBlock = blockArray.lastBlock();
    EXPECT_EQ((lastBlock != nullptr), true);

    const Block *block = blockArray.at(0);
    EXPECT_EQ((block != nullptr), true);
}

TEST_F(UT_BlockArray_Test, len)
{
    BlockArray blockArray;
    blockArray.setSize(100);

    size_t res = blockArray.newBlock();
    EXPECT_EQ(res > 0, true);

    size_t size = blockArray.len();
    EXPECT_EQ(size > 0, true);
}

TEST_F(UT_BlockArray_Test, has)
{
    BlockArray blockArray;

    size_t res = blockArray.newBlock();
    EXPECT_EQ(res > 0, true);

    bool hasBlock = blockArray.has(0);
    EXPECT_EQ(hasBlock, true);
}

TEST_F(UT_BlockArray_Test, getCurrent)
{
    BlockArray blockArray;

    size_t res = blockArray.newBlock();
    EXPECT_EQ(res > 0, true);

    size_t current = blockArray.getCurrent();
    EXPECT_EQ(current, size_t(-1));
}

TEST_F(UT_BlockArray_Test, increaseBuffer)
{
    BlockArray blockArray;

    size_t res = blockArray.newBlock();
    EXPECT_EQ(res > 0, true);

    //直接调用increaseBuffer/decreaseBuffer会crash
    blockArray.setHistorySize(100);
    blockArray.setHistorySize(1000);
}

TEST_F(UT_BlockArray_Test, decreaseBuffer)
{
    BlockArray blockArray;

    size_t res = blockArray.newBlock();
    EXPECT_EQ(res > 0, true);

    blockArray.setHistorySize(1000);
    blockArray.setHistorySize(500);
}

#endif
