project(unit-test LANGUAGES CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

enable_testing(true)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

configure_file(../src/environments.h.in environments.h @ONLY)

find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Test REQUIRED)

find_package(DtkWidget REQUIRED dtkwidget)
find_package(DtkGui REQUIRED dtkgui)
find_package(DtkCore REQUIRED dtkcore)

set(INCLUDE_DIRS
    ${Qt5Core_LIBRARIES}
    ${Qt5Gui_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Core_PRIVATE_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${ATSPI2_INCLUDE_DIRS}
    ${GOBJECT_INCLUDE_DIRS})

set (DTNG_LIB_CPP_FILES
    ../src/common/utils.cpp
    ../src/customcommand/customcommandoptdlg.cpp
    ../src/customcommand/customcommandpanel.cpp
    ../src/customcommand/customcommandplugin.cpp
    ../src/customcommand/customcommandsearchrstpanel.cpp
    ../src/customcommand/customcommandtoppanel.cpp
    ../src/encodeplugin/encodepanelplugin.cpp
    ../src/encodeplugin/encodepanel.cpp
    ../src/encodeplugin/encodelistview.cpp
    ../src/encodeplugin/encodelistmodel.cpp
    ../src/main/service.cpp
    ../src/main/windowsmanager.cpp
    ../src/main/mainwindow.cpp
    ../src/main/terminalapplication.cpp
    ../src/main/termproperties.cpp
    ../src/main/dbusmanager.cpp
    ../src/main/atspidesktop.cpp
    ../src/quaketerminal/quaketerminaladapter.cpp
    ../src/quaketerminal/quaketerminalproxy.cpp
    ../src/quaketerminal/termargumentparser.cpp
    ../src/remotemanage/remotemanagementpanel.cpp
    ../src/remotemanage/remotemanagementplugn.cpp
    ../src/remotemanage/remotemanagementsearchpanel.cpp
    ../src/remotemanage/remotemanagementtoppanel.cpp
    ../src/remotemanage/serverconfiggrouppanel.cpp
    ../src/remotemanage/serverconfigmanager.cpp
    ../src/remotemanage/serverconfigoptdlg.cpp
    ../src/settings/newdspinbox.cpp
    ../src/settings/settings.cpp
    ../src/settings/settings_translation.cpp
    ../src/settings/shortcutmanager.cpp
    ../src/theme/themeitemdelegate.cpp
    ../src/theme/themelistmodel.cpp
    ../src/theme/themelistview.cpp
    ../src/theme/themepanel.cpp
    ../src/theme/themepanelplugin.cpp
    ../src/views/commonpanel.cpp
    ../src/views/myiconbutton.cpp
    ../src/views/warnningdlg.cpp
    ../src/views/operationconfirmdlg.cpp
    ../src/views/pagesearchbar.cpp
    ../src/views/rightpanel.cpp
    ../src/views/tabbar.cpp
    ../src/views/termwidget.cpp
    ../src/views/termwidgetpage.cpp
    ../src/views/termbasedialog.cpp
    ../src/views/terminputdialog.cpp
    ../src/views/termcommandlinkbutton.cpp
    ../src/views/titlebar.cpp
    ../src/views/itemwidget.cpp
    ../src/views/focusframe.cpp
    ../src/views/iconbutton.cpp
    ../src/views/listview.cpp
)

set (DTNG_LIB_HEADER_FILES
    ../src/common/define.h
    ../src/common/utils.h
    ../src/customcommand/customcommandoptdlg.h
    ../src/customcommand/customcommandpanel.h
    ../src/customcommand/customcommandplugin.h
    ../src/customcommand/customcommandsearchrstpanel.h
    ../src/customcommand/customcommandtoppanel.h
    ../src/encodeplugin/encodepanelplugin.h
    ../src/encodeplugin/encodepanel.h
    ../src/encodeplugin/encodelistview.h
    ../src/encodeplugin/encodelistmodel.h
    ../src/main/windowsmanager.h
    ../src/main/mainwindow.h
    ../src/main/mainwindowplugininterface.h
    ../src/main/service.h
    ../src/main/terminalapplication.h
    ../src/main/termproperties.h
    ../src/main/dbusmanager.h
    ../src/main/atspidesktop.h
    ../src/quaketerminal/quaketerminaladapter.h
    ../src/quaketerminal/quaketerminalproxy.h
    ../src/quaketerminal/termargumentparser.h
    ../src/remotemanage/remotemanagementpanel.h
    ../src/remotemanage/remotemanagementplugn.h
    ../src/remotemanage/remotemanagementsearchpanel.h
    ../src/remotemanage/remotemanagementtoppanel.h
    ../src/remotemanage/serverconfiggrouppanel.h
    ../src/remotemanage/serverconfigmanager.h
    ../src/remotemanage/serverconfigoptdlg.h
    ../src/settings/newdspinbox.h
    ../src/settings/settings.h
    ../src/settings/shortcutmanager.h
    ../src/theme/themeitemdelegate.h
    ../src/theme/themelistmodel.h
    ../src/theme/themelistview.h
    ../src/theme/themepanel.h
    ../src/theme/themepanelplugin.h
    ../src/views/commonpanel.h
    ../src/views/myiconbutton.h
    ../src/views/operationconfirmdlg.h
    ../src/views/warnningdlg.h
    ../src/views/pagesearchbar.h
    ../src/views/rightpanel.h
    ../src/views/tabbar.h
    ../src/views/termwidget.h
    ../src/views/termwidgetpage.h
    ../src/views/titlebar.h
    ../src/views/termbasedialog.h
    ../src/views/terminputdialog.h
    ../src/views/termcommandlinkbutton.h
    ../src/views/itemwidget.h
    ../src/views/focusframe.h
    ../src/views/iconbutton.h
    ../src/views/listview.h
)

set (DTNG_LIB_QRC_FILES
    ../src/assets/resources.qrc
)

include_directories("../src/common")
include_directories("../src/customcommand")
include_directories("../src/encodeplugin")
include_directories("../src/main")
include_directories("../src/quaketerminal")
include_directories("../src/remotemanage")
include_directories("../src/settings")
include_directories("../src/theme")
include_directories("../src/views")
include_directories("../3rdparty/terminalwidget/lib")

include_directories("googletest/googletest/include/")

find_package(Qt5Test REQUIRED)

add_definitions(${Qt5Test_DEFINITIONS})

set (LIB_NAME deepin-terminal-lib)
add_library(${LIB_NAME} OBJECT ${DTNG_LIB_CPP_FILES} ${DTNG_LIB_HEADER_FILES} ${DTNG_LIB_QRC_FILES})
target_link_libraries(${LIB_NAME} PUBLIC ${LINK_LIBS} terminalwidget5)
target_include_directories(${LIB_NAME} PUBLIC ${Qt5Widgets_LIBRARIES}
                                              ${Qt5DBus_LIBRARIES}
                                              ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
                                              ${Qt5Core_PRIVATE_INCLUDE_DIRS}
                                              ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
                                              ${PROJECT_BINARY_DIR}
                                              ${DtkWidget_INCLUDE_DIRS}
                                              ${DtkCore_LIBRARIES}
                                              ${DtkGUI_INCLUDE_DIRS}
                                              ${DFrameworkDBus_INCLUDE_DIRS}
                                              ${ATSPI2_INCLUDE_DIRS}
                                              ${GOBJECT_INCLUDE_DIRS})
#GTest & GMock
add_subdirectory(googletest)

set(LINK_LIBS
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    Qt5::WidgetsPrivate
    Qt5::Network
    Qt5::Test
    Qt5::Gui
    gmock
    gtest

    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    ${ATSPI2_LIBRARIES}
    ${GOBJECT_LIBRARIES}
)

#Utils
add_executable(ut_utils_test test/common/ut_utils_test.cpp)
add_test(ut_utils_test COMMAND ut_utils_test)
target_include_directories(ut_utils_test PUBLIC ${INCLUDE_DIRS})
target_link_libraries(ut_utils_test PUBLIC ${LIB_NAME} ${LINK_LIBS})

#自定义命令
add_executable(ut_customcommandpanel_test test/customcommand/ut_customcommandpanel_test.cpp)
add_test(ut_customcommandpanel_test COMMAND ut_customcommandpanel_test)
target_include_directories(ut_customcommandpanel_test PUBLIC ${INCLUDE_DIRS})
target_link_libraries(ut_customcommandpanel_test PUBLIC ${LIB_NAME} ${LINK_LIBS})
