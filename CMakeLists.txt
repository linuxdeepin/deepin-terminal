project(dtng)

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
find_package(Qt5 ${QT_MINIMUM_VERSION} CONFIG REQUIRED Widgets LinguistTools)
find_package(Dtk REQUIRED Core Widget)
find_package(QTermWidget5 REQUIRED)

# Populate a CMake variable with the sources
# TODO: portable headers?
set (DTNG_CPP_FILES
    main.cpp
    mainwindow.cpp
    tabbar.cpp
    termwidget.cpp
    termwidgetpage.cpp
    titlebar.cpp
    rightpanel.cpp
    themepanel.cpp
    themelistview.cpp
    themelistmodel.cpp
    themeitemdelegate.cpp
    termproperties.cpp
    settings.cpp
)

set (DTNG_HEADER_FILES
    define.h
    mainwindow.h
    tabbar.h
    termwidget.h
    termwidgetpage.h
    titlebar.h
    rightpanel.h
    themepanel.h
    themelistview.h
    themelistmodel.h
    themeitemdelegate.h
    termproperties.h
    settings.h
)

set (DTNG_QRC_FILES
    resources.qrc
)

# Program
set (EXE_NAME dterm)

# Translation
file (GLOB DTNG_TS_FILES translations/*.ts)
set (DTNG_CPP_FILES_FOR_I18N ${DTNG_CPP_FILES})

dtk_create_i18n_from_json(DTNG_SETTINGS_I18N_FILES default-config.json settings_translation.cpp)
list (APPEND DTNG_CPP_FILES_FOR_I18N ${DTNG_SETTINGS_I18N_FILES})
qt5_create_translation(DTNG_QM_FILES ${DTNG_CPP_FILES_FOR_I18N} translations/dterm.ts)

add_executable (${EXE_NAME}
    ${DTNG_HEADER_FILES}
    ${DTNG_CPP_FILES}
    ${DTNG_QRC_FILES}
    ${DTNG_QM_FILES}
)

target_include_directories(${EXE_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${QTERMWIDGET_INCLUDE_DIRS}
)

target_link_libraries (dterm Qt5::Widgets ${DtkWidget_LIBRARIES} qtermwidget5)

# Compile definitions for QTermWidget
# So we can use QT_VERSION_CHECK
target_compile_definitions(${EXE_NAME} PRIVATE
    QTERMWIDGET_VERSION_MAJOR=${QTermWidget5_VERSION_MAJOR}
    QTERMWIDGET_VERSION_MINOR=${QTermWidget5_VERSION_MINOR}
    QTERMWIDGET_VERSION_PATCH=${QTermWidget5_VERSION_PATCH}
)

# Install settings

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

install(FILES ${DTNG_QM_FILES} DESTINATION share/dterm/translations)
