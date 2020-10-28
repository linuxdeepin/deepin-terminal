
project(deepin-terminal-test)

configure_file(../src/environments.h.in environments.h @ONLY)

# 设置添加gocv相关信息的输出
set(CMAKE_CXX_FLAGS "-g -fprofile-arcs -ftest-coverage")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

configure_file(../src/environments.h.in environments.h @ONLY)

find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5LinguistTools REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Test REQUIRED)
add_definitions(${Qt5Test_DEFINITIONS})

include(FindPkgConfig)

pkg_search_module(DtkWidget REQUIRED dtkwidget)
pkg_search_module(DtkGui REQUIRED dtkgui)
pkg_search_module(DtkCore REQUIRED dtkcore)
pkg_search_module(GOBJECT REQUIRED gobject-2.0)
pkg_check_modules(ATSPI2 REQUIRED atspi-2)
pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)
pkg_check_modules(LIBSECRET REQUIRED libsecret-1)

include_directories(${DtkWidget_INCLUDE_DIRS})
include_directories(${DtkGui_INCLUDE_DIRS})
include_directories(${DtkCore_INCLUDE_DIRS})

include_directories("../src/common")
include_directories("../src/customcommand")
include_directories("../src/encodeplugin")
include_directories("../src/main")
include_directories("../src/remotemanage")
include_directories("../src/settings")
include_directories("../src/theme")
include_directories("../src/views")
include_directories("../3rdparty/terminalwidget/lib")

include_directories("src/common")
include_directories("src/3rdparty/terminalwidget/lib")

set(TERMINALWIDGET_VERSION_MAJOR "0")
set(TERMINALWIDGET_VERSION_MINOR "14")
set(TERMINALWIDGET_VERSION_PATCH "1")

add_subdirectory(termwidget5)

set(LINK_LIBS
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    Qt5::WidgetsPrivate
    Qt5::Network
    Qt5::Test

    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    ${ATSPI2_LIBRARIES}
    ${GOBJECT_LIBRARIES}
    ${LIBSECRET_LIBRARIES}
)

FILE(GLOB allSource
    ../src/common/*.cpp
    ../src/customcommand/*.cpp
    ../src/encodeplugin/*.cpp
    ../src/quaketerminal/*.cpp
    ../src/remotemanage/*.cpp
    ../src/settings/*.cpp
    ../src/theme/*.cpp
    ../src/views/*.cpp
    ../src/main/service.cpp
    ../src/main/windowsmanager.cpp
    ../src/main/mainwindow.cpp
    ../src/main/terminalapplication.cpp
    ../src/main/termproperties.cpp
    ../src/main/dbusmanager.cpp
    ../src/main/atspidesktop.cpp
)

FILE(GLOB allHeaders
    ../src/common/*.h
    ../src/customcommand/*.h
    ../src/encodeplugin/*.h
    ../src/main/*.h
    ../src/quaketerminal/*.h
    ../src/remotemanage/*.h
    ../src/settings/*.h
    ../src/theme/*.h
    ../src/views/*.h
)

FILE(GLOB qrcFiles
    ../src/assets/resources.qrc
)

include_directories(googletest)
include_directories(googletest/googlemock/include)
include_directories(googletest/googletest/include)

#GTest & GMock
add_subdirectory(googletest)

set(PROJECT_NAME_TEST ${PROJECT_NAME})

FILE(GLOB allTestSource
    src/common/*.cpp
    src/customcommand/*.cpp
    src/encodeplugin/*.cpp
    src/main/*.cpp
    src/remotemanage/*.cpp
    src/settings/*.cpp
    src/theme/*.cpp
    src/views/*.cpp
    src/3rdparty/terminalwidget/lib/*.cpp
)

add_executable(${PROJECT_NAME_TEST}
    ${allHeaders}
    ${allSource}
    ${allTestSource}
    ${qrcFiles}
    ${uiFiles}
)

target_include_directories(${PROJECT_NAME_TEST} PUBLIC ${Qt5Widgets_LIBRARIES}
                                                        ${Qt5DBus_LIBRARIES}
                                                        ${Qt5TestLib_LIBRARIES}
                                                        ${Qt5Widgets_PRIVATE_INCLUDE_DIRS}
                                                        ${Qt5Core_PRIVATE_INCLUDE_DIRS}
                                                        ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
                                                        ${PROJECT_BINARY_DIR}
                                                        ${DtkWidget_INCLUDE_DIRS}
                                                        ${DtkCore_LIBRARIES}
                                                        ${DtkGUI_INCLUDE_DIRS}
                                                        ${DFrameworkDBus_INCLUDE_DIRS}
                                                        ${ATSPI2_INCLUDE_DIRS}
                                                        ${GOBJECT_INCLUDE_DIRS}
                                                        ${LIBSECRET_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME_TEST} PUBLIC ${LINK_LIBS} termwidget5 gmock gmock_main gtest gtest_main)

#------------------------------ 创建'make test'指令---------------------------------------
add_custom_target(test
    #执行mkdir -p 创建 coverageResult
    COMMAND mkdir -p coverageResult
    #设置工作目录
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_command(TARGET test
    COMMAND echo " =================== CREAT LCOV REPROT BEGIN ==================== "

    #1.清理原先的gcov信息
    COMMAND lcov --directory ./CMakeFiles/${PROJECT_NAME_TEST}.dir --zerocounters
    COMMAND lcov --directory ./CMakeFiles/${PROJECT_NAME}.dir --zerocounters
    COMMAND ./test/${PROJECT_NAME_TEST}

    #2.收集gcov信息到.info文件中
    COMMAND lcov --directory . --capture --output-file ./coverageResult/${PROJECT_NAME}_Coverage.info

    #3.过滤一些我们不感兴趣的文件的覆盖率信息
    COMMAND echo " =================== do filter begin ==================== "
    COMMAND lcov --remove ./coverageResult/${PROJECT_NAME}_Coverage.info
    '*/${PROJECT_NAME_TEST}_autogen/*' '*/${PROJECT_NAME}_autogen/*' '*/usr/include/*' '*/theme/*' '*/quaketerminal/*' '*/test/*' 'qtermwidget.h' '*/lib/SearchBar.*' '*/build-*/*' '*/googletest/*'
    -o ./coverageResult/${PROJECT_NAME}_Coverage.info
    COMMAND echo " =================== do filter end ==================== "

    #3.将.info信息生成报告到reprot文件夹中
    COMMAND genhtml -o ./coverageResult/report ./coverageResult/${PROJECT_NAME}_Coverage.info

    COMMAND echo " =================== CREAT LCOV REPROT END ==================== "

    COMMAND echo " Coverage files have been output to ${CMAKE_BINARY_DIR}/coverageResult/report "

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

#'make test'命令依赖与我们的测试程序
add_dependencies(test ${PROJECT_NAME_TEST})

