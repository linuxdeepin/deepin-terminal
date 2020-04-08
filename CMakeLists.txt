project(deepin-terminal)

cmake_minimum_required(VERSION 3.9.5)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

include (DtkSettingsToolsMacros)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")
set(QT_MINIMUM_VERSION "5.7.1")

configure_file(environments.h.in environments.h @ONLY)

# Find the QtWidgets library

find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5LinguistTools)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Network REQUIRED)

pkg_search_module(DtkWidget REQUIRED dtkwidget)
pkg_search_module(DtkGui REQUIRED dtkgui)
pkg_search_module(DtkCore REQUIRED dtkcore)

# Compile definitions for TerminalWidget
# So we can use QT_VERSION_CHECK
set(TERMINALWIDGET_VERSION_MAJOR ${TerminalWidget5_VERSION_MAJOR})
set(TERMINALWIDGET_VERSION_MINOR=${TerminalWidget5_VERSION_MINOR})
set(TERMINALWIDGET_VERSION_PATCH=${TerminalWidget5_VERSION_PATCH})

add_subdirectory(terminalwidget)

include_directories(${DtkWidget_INCLUDE_DIRS})
include_directories(${DtkGui_INCLUDE_DIRS})
include_directories(${DtkCore_INCLUDE_DIRS})

include_directories("common")
include_directories("customcommand")
include_directories("encodeplugin")
include_directories("main")
include_directories("quaketerminal")
include_directories("remotemanage")
include_directories("settings")
include_directories("theme")
include_directories("views")

set(LINK_LIBS
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    Qt5::WidgetsPrivate
    Qt5::Network

    ${DtkWidget_LDFLAGS}
    ${DtkGui_LDFLAGS}
    ${DtkCore_LDFLAGS}
)

# Populate a CMake variable with the sources
# TODO: portable headers?
set (DTNG_CPP_FILES
    common/utils.cpp
    common/termqsettings.cpp
    customcommand/customcommanddelegate.cpp
    customcommand/customcommanditemmodel.cpp
    customcommand/customcommandlist.cpp
    customcommand/customcommandoptdlg.cpp
    customcommand/customcommandpanel.cpp
    customcommand/customcommandplugin.cpp
    customcommand/customcommandsearchrstpanel.cpp
    customcommand/customcommandtoppanel.cpp
    encodeplugin/encodepanelplugin.cpp
    encodeplugin/encodepanel.cpp
    encodeplugin/encodelistview.cpp
    encodeplugin/encodelistmodel.cpp
    encodeplugin/encodeitemdelegate.cpp
    main/main.cpp
    main/mainwindow.cpp
    main/termproperties.cpp
    main/dterminalsingleapplication.cpp
    quaketerminal/quaketerminaladapter.cpp
    quaketerminal/quaketerminalproxy.cpp
    quaketerminal/termargumentparser.cpp
    remotemanage/remotemanagementpanel.cpp
    remotemanage/remotemanagementplugn.cpp
    remotemanage/remotemanagementsearchpanel.cpp
    remotemanage/remotemanagementtoppanel.cpp
    remotemanage/serverconfigdelegate.cpp
    remotemanage/serverconfiggrouppanel.cpp
    remotemanage/serverconfigitem.cpp
    remotemanage/serverconfigitemmodel.cpp
    remotemanage/serverconfiglist.cpp
    remotemanage/serverconfigmanager.cpp
    remotemanage/serverconfigoptdlg.cpp
    remotemanage/serverconfigoptwidget.cpp
    settings/newdspinbox.cpp
    settings/settings.cpp
    settings/settings_translation.cpp
    settings/shortcutmanager.cpp
    theme/themeitemdelegate.cpp
    theme/themelistmodel.cpp
    theme/themelistview.cpp
    theme/themepanel.cpp
    theme/themepanelplugin.cpp
    views/commonpanel.cpp
    views/myiconbutton.cpp
    views/operationconfirmdlg.cpp
    views/pagesearchbar.cpp
    views/rightpanel.cpp
    views/tabbar.cpp
    views/termwidget.cpp
    views/termwidgetpage.cpp
    views/termbasedialog.cpp
    views/termcommandlinkbutton.cpp
    views/titlebar.cpp
)

set (DTNG_HEADER_FILES
    common/define.h
    common/termqsettings.h
    common/termqsettings_p.h
    common/utils.h
    customcommand/customcommanddelegate.h
    customcommand/customcommanditemmodel.h
    customcommand/customcommandlist.h
    customcommand/customcommandoptdlg.h
    customcommand/customcommandpanel.h
    customcommand/customcommandplugin.h
    customcommand/customcommandsearchrstpanel.h
    customcommand/customcommandtoppanel.h
    encodeplugin/encodepanelplugin.h
    encodeplugin/encodepanel.h
    encodeplugin/encodelistview.h
    encodeplugin/encodelistmodel.h
    encodeplugin/encodeitemdelegate.h
    main/mainwindow.h
    main/mainwindowplugininterface.h
    main/termproperties.h
    main/dterminalsingleapplication.h
    quaketerminal/quaketerminaladapter.h
    quaketerminal/quaketerminalproxy.h
    quaketerminal/termargumentparser.h
    remotemanage/remotemanagementpanel.h
    remotemanage/remotemanagementplugn.h
    remotemanage/remotemanagementsearchpanel.h
    remotemanage/remotemanagementtoppanel.h
    remotemanage/serverconfigdelegate.h
    remotemanage/serverconfiggrouppanel.h
    remotemanage/serverconfigitem.h
    remotemanage/serverconfigitemmodel.h
    remotemanage/serverconfiglist.h
    remotemanage/serverconfigmanager.h
    remotemanage/serverconfigoptdlg.h
    remotemanage/serverconfigoptwidget.h
    settings/newdspinbox.h
    settings/settings.h
    settings/shortcutmanager.h
    theme/themeitemdelegate.h
    theme/themelistmodel.h
    theme/themelistview.h
    theme/themepanel.h
    theme/themepanelplugin.h
    views/commonpanel.h
    views/myiconbutton.h
    views/operationconfirmdlg.h
    views/pagesearchbar.h
    views/rightpanel.h
    views/tabbar.h
    views/termwidget.h
    views/termwidgetpage.h
    views/titlebar.h
    views/termbasedialog.h
    views/termcommandlinkbutton.h
)

set (DTNG_QRC_FILES
    resources.qrc
)

# Program
set (EXE_NAME deepin-terminal)

# Translation
file (GLOB DTNG_TS_FILES translations/*.ts)
set (DTNG_CPP_FILES_FOR_I18N ${DTNG_CPP_FILES})

dtk_create_i18n_from_json(DTNG_SETTINGS_I18N_FILES default-config.json settings_translation.cpp)
list (APPEND DTNG_CPP_FILES_FOR_I18N ${DTNG_SETTINGS_I18N_FILES})
qt5_create_translation(DTNG_QM_FILES ${DTNG_CPP_FILES_FOR_I18N} ${DTNG_TS_FILES})

add_executable (${EXE_NAME}
    ${DTNG_HEADER_FILES}
    ${DTNG_CPP_FILES}
    ${DTNG_QRC_FILES}
    ${DTNG_QM_FILES}
)

target_link_libraries (deepin-terminal ${LINK_LIBS} terminalwidget5)

# Install settings

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

# Installation

#install(TARGETS deepin-terminal DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-terminal)

install(FILES deepin-terminal.desktop
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications/)

install(FILES resources/images/logo/deepin-terminal.svg
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/)

# add this empty file for first install/reinstall judge
install(FILES resources/other/install_flag
        DESTINATION $ENV{HOME}/.config/deepin/deepin-terminal/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/deepin-terminal
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        PERMISSIONS OWNER_READ OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

install(FILES ${DTNG_QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-terminal/translations)
