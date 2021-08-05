/*
 *  Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
 *
 * Author:     wangliang <wangliang@uniontech.com>
 *
 * Maintainer: wangliang <wangliang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "ut_defines.h"
#include "settings.h"
#include "../stub.h"

//Qt单元测试相关头文件
#include <QTest>
#include <QtGui>
#include <QDebug>
#include <QFile>
#include <QMenu>

#include <DSettings>
#include <DSettingsOption>

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

TEST_F(UT_Settings_Test, init)
{
    Stub stub;
    stub.set((QString (QVariant::*)() const)ADDR(QVariant, toString), ut_variant_toString);
    stub.set((bool (QFile::*)() const)ADDR(QFile, exists), ut_file_exists);

    Settings::releaseInstance();
    Settings::instance();

}
TEST_F(UT_Settings_Test, SettingsTest)
{
    Settings::instance()->opacity();

    Settings::instance()->fontName();

    Settings::instance()->fontSize();

    //由于编码没有保存到conf，这里只验证和默认编码相同
    Settings::instance()->encoding();

    Settings::instance()->cursorShape();

    Settings::instance()->cursorBlink();

    Settings::instance()->backgroundBlur();

    Settings::instance()->colorScheme();

    Settings::instance()->PressingScroll();

    Settings::instance()->OutputtingScroll();
}

TEST_F(UT_Settings_Test, GenerateSettingTranslate)
{
    GenerateSettingTranslate();
}

TEST_F(UT_Settings_Test, createSpinButtonHandle)
{
    DSettingsOption option;
    option.setData("min", QVariant(10));
    option.setData("max", QVariant(90));
    Settings::instance()->createSpinButtonHandle(&option);
}

TEST_F(UT_Settings_Test, createCustomSliderHandle)
{
    DSettingsOption option;
    option.setData("min", QVariant(0));
    option.setData("max", QVariant(100));
    Settings::instance()->createCustomSliderHandle(&option);
}

TEST_F(UT_Settings_Test, createFontComBoBoxHandle)
{
    Stub stub;
    stub.set((int (QStringList::*)() const)ADDR(QStringList, size), ut_stringlist_size);

    DSettingsOption option;
    Settings::instance()->createFontComBoBoxHandle(&option);
}

TEST_F(UT_Settings_Test, createShortcutEditOptionHandle)
{
    DSettingsOption option;
    option.setValue(QVariant("Ctrl+Shift+A"));
    KeySequenceEdit *w = static_cast<KeySequenceEdit *>(Settings::instance()->createShortcutEditOptionHandle(&option).second);
    if(!w)
        return;

    emit w->editingFinished(QKeySequence("Backspace"));
    emit w->editingFinished(QKeySequence("Esc"));
    emit w->editingFinished(QKeySequence("F4"));
    emit w->editingFinished(QKeySequence(""));
}

TEST_F(UT_Settings_Test, createShellConfigComboxOptionHandle)
{
    Stub stub;
    stub.set((bool (QFile::*)() const)ADDR(QFile, exists), ut_file_exists);

    QSharedPointer<DSettingsOption> option(new DSettingsOption, doDeleteLater);
    option->setValue(QVariant("Ctrl+Shift+A"));
    DComboBox *w = static_cast<DComboBox *>(Settings::instance()->createShellConfigComboxOptionHandle(option.data()).second);
    if(!w)
        return;

    const QString DEFAULT_SHELL = "$SHELL";
    emit option->valueChanged(DEFAULT_SHELL);
    emit option->valueChanged("sh");
    emit w->currentTextChanged("zsh");
}

TEST_F(UT_Settings_Test, loadDefaultsWhenReinstall)
{
    Settings::instance()->loadDefaultsWhenReinstall();

    emit Settings::instance()->settings->valueChanged("", "");
}
TEST_F(UT_Settings_Test, handleWidthFont)
{
    Stub stub;
    stub.set((int (QComboBox::*)(const QString &, Qt::MatchFlags) const)ADDR(DComboBox, findText), ut_combobox_findText);

    Settings::instance()->handleWidthFont();
    Settings::instance()->enableControlFlow();
}


#endif
