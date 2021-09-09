project(untitled82)

cmake_minimum_required(VERSION 3.9.5)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(PkgConfig REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)
find_package(DtkCore REQUIRED)

add_subdirectory(qtermwidget)

add_executable(${PROJECT_NAME}
    "main.cpp"
    "mainwindow.cpp"
    "mainwindow.h"
    )

target_include_directories(${PROJECT_NAME} PUBLIC
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkGUI_INCLUDE_DIRS}
    ${DtkCore_INCLUDE_DIRS}
    )

target_link_libraries(${PROJECT_NAME}
    ${Qt5Gui_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Core_LIBRARIES}

    ${DtkWidget_LIBRARIES}
    ${DtkGUI_LIBRARIES}
    ${DtkCore_LIBRARIES}
    qtermwidget5
    )
