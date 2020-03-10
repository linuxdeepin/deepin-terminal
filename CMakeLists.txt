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

# Find the QtWidgets library

find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5LinguistTools)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5LinguistTools REQUIRED)

pkg_search_module(DtkWidget REQUIRED dtkwidget)
pkg_search_module(DtkGui REQUIRED dtkgui)
pkg_search_module(DtkCore REQUIRED dtkcore)

#find_package(QTermWidget5 REQUIRED)

# Compile definitions for QTermWidget
# So we can use QT_VERSION_CHECK
set(QTERMWIDGET_VERSION_MAJOR ${QTermWidget5_VERSION_MAJOR})
set(QTERMWIDGET_VERSION_MINOR=${QTermWidget5_VERSION_MINOR})
set(QTERMWIDGET_VERSION_PATCH=${QTermWidget5_VERSION_PATCH})

add_subdirectory(qtermwidget)

include_directories(${DtkWidget_INCLUDE_DIRS})
include_directories(${DtkGui_INCLUDE_DIRS})
include_directories(${DtkCore_INCLUDE_DIRS})

set(LINK_LIBS
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    Qt5::WidgetsPrivate

    ${DtkWidget_LDFLAGS}
    ${DtkGui_LDFLAGS}
    ${DtkCore_LDFLAGS}
)

# Populate a CMake variable with the sources
# TODO: portable headers?
set (DTNG_CPP_FILES
    commonpanel.cpp
    customcommanddelegate.cpp
    customcommanditemmodel.cpp
    customcommandlist.cpp
    customcommandoptdlg.cpp
    customcommandpanel.cpp
    customcommandplugin.cpp
    customcommandsearchrstpanel.cpp
    customcommandtoppanel.cpp
    main.cpp
    mainwindow.cpp
    myiconbutton.cpp
    operationconfirmdlg.cpp
    quaketerminaladapter.cpp
    quaketerminalproxy.cpp
    remotemanagementpanel.cpp
    remotemanagementplugn.cpp
    remotemanagementsearchpanel.cpp
    remotemanagementtoppanel.cpp
    rightpanel.cpp
    serverconfigdelegate.cpp
    serverconfiggrouppanel.cpp
    serverconfigitem.cpp
    serverconfigitemmodel.cpp
    serverconfiglist.cpp
    serverconfigmanager.cpp
    serverconfigoptdlg.cpp
    serverconfigoptwidget.cpp
    settings.cpp
    settings_translation.cpp
    shortcutmanager.cpp
    tabbar.cpp
    termargumentparser.cpp
    termproperties.cpp
    termtabbar.cpp
    termwidget.cpp
    termwidgetpage.cpp
    themeitemdelegate.cpp
    themelistmodel.cpp
    themelistview.cpp
    themepanel.cpp
    themepanelplugin.cpp
    titlebar.cpp
    utils.cpp
    pagesearchbar.cpp
    termqsettings.cpp
    termbasedialog.cpp
    termcommandlinkbutton.cpp
    encodeplugin/encodepanelplugin.cpp
    encodeplugin/encodepanel.cpp
    encodeplugin/encodelistview.cpp
    encodeplugin/encodelistmodel.cpp
    encodeplugin/encodeitemdelegate.cpp
)

set (DTNG_HEADER_FILES
    commonpanel.h
    customcommanddelegate.h
    customcommanditemmodel.h
    customcommandlist.h
    customcommandoptdlg.h
    customcommandpanel.h
    customcommandplugin.h
    customcommandsearchrstpanel.h
    customcommandtoppanel.h
    define.h
    mainwindow.h
    mainwindowplugininterface.h
    myiconbutton.h
    operationconfirmdlg.h
    quaketerminaladapter.h
    quaketerminalproxy.h
    remotemanagementpanel.h
    remotemanagementplugn.h
    remotemanagementsearchpanel.h
    remotemanagementtoppanel.h
    rightpanel.h
    serverconfigdelegate.h
    serverconfiggrouppanel.h
    serverconfigitem.h
    serverconfigitemmodel.h
    serverconfiglist.h
    serverconfigmanager.h
    serverconfigoptdlg.h
    serverconfigoptwidget.h
    settings.h
    shortcutmanager.h
    tabbar.h
    termargumentparser.h
    termproperties.h
    termtabbar.h
    termwidget.h
    termwidgetpage.h
    themeitemdelegate.h
    themelistmodel.h
    themelistview.h
    themepanel.h
    themepanelplugin.h
    titlebar.h
    utils.h
    pagesearchbar.h
    termqsettings.h
    termqsettings_p.h
    termbasedialog.h
    termcommandlinkbutton.h
    encodeplugin/encodepanelplugin.h
    encodeplugin/encodepanel.h
    encodeplugin/encodelistview.h
    encodeplugin/encodelistmodel.h
    encodeplugin/encodeitemdelegate.h
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

target_link_libraries (deepin-terminal ${LINK_LIBS} qtermwidget5)

# Install settings

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

# Installation

#install(TARGETS deepin-terminal DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-terminal)

install(FILES deepin-terminal.desktop
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications/)

install(FILES images/logo/deepin-terminal.svg
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/deepin-terminal
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        PERMISSIONS OWNER_READ OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE)

install(FILES ${DTNG_QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/deepin-terminal/translations)
