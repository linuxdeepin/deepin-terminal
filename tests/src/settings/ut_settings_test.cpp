// Copyright (C) 2019 ~ 2023 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_defines.h"
#include "settings.h"
#include "../stub.h"
#include "ut_stub_defines.h"
#include "dbusmanager.h"

//dtk
#include <DSettings>
#include <DSettingsOption>

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QFile>
#include <QMenu>
#include <QSignalSpy>

//google test
#include <gtest/gtest.h>

DCORE_USE_NAMESPACE

extern void GenerateSettingTranslate();


class UT_Settings_Test : public ::testing::Test
{
public:
    UT_Settings_Test() {}

public:
    //这里的几个函数都会自动调用

    //用于做一些初始化操作
    virtual void SetUp() {}

    //用于做一些清理操作
    virtual void TearDown() {}
};

static bool ut_file_exists()
{
    return false;
}

static int ut_combobox_findText(const QString &, Qt::MatchFlags)
{
    return -1;
}

static int ut_stringlist_size()
{
    return 0;
}

static QString ut_variant_toString()
{
    return QString();
}

static void doDeleteLater(DSettingsOption *obj)
{
    obj->deleteLater();
}

#ifdef UT_SETTINGS_TEST

TEST_F(UT_Settings_Test, SettingsTest)
{
    //测试场景：读取的settings的值
//    Settings::instance()->bSwitchTheme = true;

    UT_STUB_DSETTINGSOPTION_VALUE_CREATE;
    Settings::instance()->opacity();
    EXPECT_TRUE(UT_STUB_DSETTINGSOPTION_VALUE_RESULT);

    UT_STUB_DSETTINGSOPTION_VALUE_PREPARE;
    Settings::instance()->fontName();
    EXPECT_TRUE(UT_STUB_DSETTINGSOPTION_VALUE_RESULT);

    UT_STUB_DSETTINGSOPTION_VALUE_PREPARE;
    Settings::instance()->fontSize();
    EXPECT_TRUE(UT_STUB_DSETTINGSOPTION_VALUE_RESULT);

    EXPECT_TRUE(Settings::instance()->encoding() == Settings::instance()->m_EncodeName);

    UT_STUB_DSETTINGSOPTION_VALUE_PREPARE;
    Settings::instance()->cursorShape();
    EXPECT_TRUE(UT_STUB_DSETTINGSOPTION_VALUE_RESULT);

    UT_STUB_DSETTINGSOPTION_VALUE_PREPARE;
    Settings::instance()->cursorBlink();
    EXPECT_TRUE(UT_STUB_DSETTINGSOPTION_VALUE_RESULT);

    UT_STUB_DSETTINGSOPTION_VALUE_PREPARE;
    Settings::instance()->backgroundBlur();
    EXPECT_TRUE(UT_STUB_DSETTINGSOPTION_VALUE_RESULT);

    UT_STUB_DSETTINGSOPTION_VALUE_PREPARE;
    Settings::instance()->colorScheme();
    EXPECT_TRUE(UT_STUB_DSETTINGSOPTION_VALUE_RESULT);

    UT_STUB_DSETTINGSOPTION_VALUE_PREPARE;
    Settings::instance()->PressingScroll();
    EXPECT_TRUE(UT_STUB_DSETTINGSOPTION_VALUE_RESULT);

    UT_STUB_DSETTINGSOPTION_VALUE_PREPARE;
    Settings::instance()->OutputtingScroll();
    EXPECT_TRUE(UT_STUB_DSETTINGSOPTION_VALUE_RESULT);
}

TEST_F(UT_Settings_Test, GenerateSettingTranslate)
{
    UT_STUB_QTRANSLATE_TRANSLATE_CREATE;
    GenerateSettingTranslate();
    EXPECT_TRUE(UT_STUB_QTRANSLATE_TRANSLATE_RESULT);
}

TEST_F(UT_Settings_Test, createSpinButtonHandle)
{
    DSettingsOption option;
    option.setData("min", QVariant(10));
    option.setData("max", QVariant(90));
    QPair<QWidget *, QWidget *> pair = Settings::instance()->createSpinButtonHandle(&option);
    EXPECT_TRUE(pair.second);
}

TEST_F(UT_Settings_Test, createCustomSliderHandle)
{
    DSettingsOption option;
    option.setData("min", QVariant(0));
    option.setData("max", QVariant(100));
    QPair<QWidget *, QWidget *> pair = Settings::instance()->createCustomSliderHandle(&option);
    EXPECT_TRUE(pair.second);
}

TEST_F(UT_Settings_Test, createFontComBoBoxHandle)
{
    Stub stub;
    stub.set((int (QStringList::*)() const)ADDR(QStringList, size), ut_stringlist_size);
    UT_STUB_QDBUS_CALL_APPEND;

    DSettingsOption option;
    QPair<QWidget *, QWidget *> pair = Settings::instance()->createFontComBoBoxHandle(&option);
    EXPECT_TRUE(pair.second);
}

TEST_F(UT_Settings_Test, createShortcutEditOptionHandle)
{
    DSettingsOption option;
    option.setValue(QVariant("Ctrl+Shift+A"));
    QPair<QWidget *, QWidget *> pair = Settings::instance()->createShortcutEditOptionHandle(&option);
    ASSERT_TRUE(pair.second);
    DKeySequenceEdit *w = qobject_cast<DKeySequenceEdit *>(pair.second);
    ASSERT_TRUE(w);

    QSignalSpy spy(w, &DKeySequenceEdit::editingFinished);
    EXPECT_TRUE(spy.count() == 0);

    emit w->editingFinished(QKeySequence("Backspace"));
    emit w->editingFinished(QKeySequence("Esc"));
    emit w->editingFinished(QKeySequence("F4"));
    emit w->editingFinished(QKeySequence(""));

    spy.wait(1000);
    EXPECT_TRUE(spy.count() == 4);
}

TEST_F(UT_Settings_Test, createShellConfigComboxOptionHandle)
{
    Stub stub;
    stub.set((bool (QFile::*)() const)ADDR(QFile, exists), ut_file_exists);

    QSharedPointer<DSettingsOption> option(new DSettingsOption, doDeleteLater);
    option->setValue(QVariant("Ctrl+Shift+A"));
    DComboBox *w = qobject_cast<DComboBox *>(Settings::instance()->createShellConfigComboxOptionHandle(option.data()).second);
    ASSERT_TRUE(w);

    QSignalSpy spy(option.get(), &DSettingsOption::valueChanged);
    EXPECT_TRUE(spy.count() == 0);
    const QString DEFAULT_SHELL = "$SHELL";
    emit option->valueChanged(DEFAULT_SHELL);
    emit option->valueChanged("sh");
    spy.wait(1000);
    EXPECT_TRUE(spy.count() == 2);

    QSignalSpy spy2(w, &DComboBox::currentTextChanged);
    EXPECT_TRUE(spy2.count() == 0);
    emit w->currentTextChanged("zsh");
    spy2.wait(1000);
    EXPECT_TRUE(spy2.count() >= 1);
}

TEST_F(UT_Settings_Test, loadDefaultsWhenReinstall)
{
    UT_STUB_QDIR_EXISTS_CREATE;
    Settings::instance()->loadDefaultsWhenReinstall();
    //会触发dir exists函数
    EXPECT_TRUE(UT_STUB_QDIR_EXISTS_RESULT);

    QSignalSpy spy(Settings::instance()->settings, &DSettings::valueChanged);
    EXPECT_TRUE(spy.count() == 0);
    emit Settings::instance()->settings->valueChanged("", "");
    spy.wait(1000);
    EXPECT_TRUE(spy.count() == 1);
}

static FontDataList ut_DBusManager_callAppearanceFont(QString)
{
    return FontDataList().appendValues({"Courier 10 Pitch", "DejaVu Sans Mono", "Hack", "Liberation Mono", "Linux Libertine Mono O", "Nimbus Mono L", "Nimbus Mono PS", "Noto Mono", "Noto Sans Mono", "Noto Sans Mono CJK JP", "Noto Sans Mono CJK KR", "Noto Sans Mono CJK SC", "Noto Sans Mono CJK TC", "等距更纱黑体 SC", "文泉驿等宽微米黑"});
}

TEST_F(UT_Settings_Test, handleWidthFont)
{
    Stub stub;
    stub.set((int (QComboBox::*)(const QString &, Qt::MatchFlags) const)ADDR(DComboBox, findText), ut_combobox_findText);

    //获取等宽字体
    Settings::instance()->handleWidthFont();
    EXPECT_TRUE(Settings::instance()->comboBox->count() > 0);
}


#endif
