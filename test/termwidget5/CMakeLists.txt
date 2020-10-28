cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
# CMP0000: Call the cmake_minimum_required() command at the beginning of the top-level
# CMakeLists.txt file even before calling the project() command.
# The cmake_minimum_required(VERSION) command implicitly invokes the cmake_policy(VERSION)
# command to specify that the current project code is written for the given range of CMake
# versions.
project(termwidget)

include(GNUInstallDirs)
include(GenerateExportHeader)
include(CMakePackageConfigHelpers)
include(CheckFunctionExists)
include(CheckIncludeFile)

option(UPDATE_TRANSLATIONS "Update source translation translations/*.ts files" OFF)
option(BUILD_EXAMPLE "Build example application. Default OFF." OFF)
option(TERMINALWIDGET_USE_UTEMPTER "Uses the libutempter library. Mainly for FreeBSD" OFF)
option(TERMINALWIDGET_BUILD_PYTHON_BINDING "Build python binding" OFF)
option(USE_UTF8PROC "Use libutf8proc for better Unicode support. Default OFF" OFF)


# just change version for releases
set(TERMINALWIDGET_VERSION_MAJOR "0")
set(TERMINALWIDGET_VERSION_MINOR "14")
set(TERMINALWIDGET_VERSION_PATCH "1")

set(TERMINALWIDGET_VERSION "${TERMINALWIDGET_VERSION_MAJOR}.${TERMINALWIDGET_VERSION_MINOR}.${TERMINALWIDGET_VERSION_PATCH}")

# additional cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_INSTALL_PREFIX "/usr")

# Minimum Versions
set(QT_MINIMUM_VERSION "5.7.1")
set(LXQTBT_MINIMUM_VERSION "0.6.0")

find_package(Qt5Widgets "${QT_MINIMUM_VERSION}" REQUIRED)
find_package(Qt5LinguistTools "${QT_MINIMUM_VERSION}" REQUIRED)
find_package(lxqt-build-tools ${LXQTBT_MINIMUM_VERSION} REQUIRED)

if(USE_UTF8PROC)
    find_package(Utf8Proc REQUIRED)
endif()

include(LXQtPreventInSourceBuilds)
include(LXQtTranslateTs)
include(LXQtCompilerSettings NO_POLICY_SCOPE)
include(LXQtCreatePkgConfigFile)

if(APPLE)
    if(CMAKE_VERSION VERSION_GREATER 3.9)
        cmake_policy(SET CMP0068 NEW)
        set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR ON)
    endif()
endif()

set(TERMWIDGET_LIBRARY_NAME termwidget5)

# main library

set(SRCS
    ../../3rdparty/terminalwidget/lib/BlockArray.cpp
    ../../3rdparty/terminalwidget/lib/ColorScheme.cpp
    ../../3rdparty/terminalwidget/lib/Emulation.cpp
    ../../3rdparty/terminalwidget/lib/Filter.cpp
    ../../3rdparty/terminalwidget/lib/History.cpp
    ../../3rdparty/terminalwidget/lib/HistorySearch.cpp
    ../../3rdparty/terminalwidget/lib/KeyboardTranslator.cpp
    ../../3rdparty/terminalwidget/lib/konsole_wcwidth.cpp
    ../../3rdparty/terminalwidget/lib/kprocess.cpp
    ../../3rdparty/terminalwidget/lib/kpty.cpp
    ../../3rdparty/terminalwidget/lib/kptydevice.cpp
    ../../3rdparty/terminalwidget/lib/kptyprocess.cpp
    ../../3rdparty/terminalwidget/lib/ProcessInfo.cpp
    ../../3rdparty/terminalwidget/lib/Pty.cpp
    ../../3rdparty/terminalwidget/lib/qtermwidget.cpp
    ../../3rdparty/terminalwidget/lib/Screen.cpp
    ../../3rdparty/terminalwidget/lib/ScreenWindow.cpp
    ../../3rdparty/terminalwidget/lib/SearchBar.cpp
    ../../3rdparty/terminalwidget/lib/Session.cpp
    ../../3rdparty/terminalwidget/lib/SessionManager.cpp
    ../../3rdparty/terminalwidget/lib/ShellCommand.cpp
    ../../3rdparty/terminalwidget/lib/TerminalCharacterDecoder.cpp
    ../../3rdparty/terminalwidget/lib/TerminalDisplay.cpp
    ../../3rdparty/terminalwidget/lib/tools.cpp
    ../../3rdparty/terminalwidget/lib/Vt102Emulation.cpp
)

# Only the Headers that need to be moc'd go here
set(HDRS
    ../../3rdparty/terminalwidget/lib/Emulation.h
    ../../3rdparty/terminalwidget/lib/Filter.h
    ../../3rdparty/terminalwidget/lib/HistorySearch.h
    ../../3rdparty/terminalwidget/lib/kprocess.h
    ../../3rdparty/terminalwidget/lib/kptydevice.h
    ../../3rdparty/terminalwidget/lib/kptyprocess.h
    ../../3rdparty/terminalwidget/lib/ProcessInfo.h
    ../../3rdparty/terminalwidget/lib/Pty.h
    ../../3rdparty/terminalwidget/lib/qtermwidget.h
    ../../3rdparty/terminalwidget/lib/ScreenWindow.h
    ../../3rdparty/terminalwidget/lib/SearchBar.h
    ../../3rdparty/terminalwidget/lib/Session.h
    ../../3rdparty/terminalwidget/lib/SessionManager.h
    ../../3rdparty/terminalwidget/lib/TerminalDisplay.h
    ../../3rdparty/terminalwidget/lib/Vt102Emulation.h
    ../src/3rdparty/terminalwidget/lib/qtermwidget_export.h
)

set(UI
    ../../3rdparty/terminalwidget/lib/SearchBar.ui
)

# for distribution
set(HDRS_DISTRIB
    ../../3rdparty/terminalwidget/lib/qtermwidget.h
    ../../3rdparty/terminalwidget/lib/Emulation.h
    ../../3rdparty/terminalwidget/lib/Filter.h
    ../../3rdparty/terminalwidget/lib/Session.h
    ../../3rdparty/terminalwidget/lib/SessionManager.h
    ../../3rdparty/terminalwidget/lib/History.h
    ../../3rdparty/terminalwidget/lib/BlockArray.h
    ../../3rdparty/terminalwidget/lib/Character.h
    ../../3rdparty/terminalwidget/lib/CharacterColor.h
    ../../3rdparty/terminalwidget/lib/TerminalDisplay.h
)

# dirs
set(KB_LAYOUT_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${TERMWIDGET_LIBRARY_NAME}/kb-layouts")
message(STATUS "Keyboard layouts will be installed in: ${KB_LAYOUT_DIR}")

set(TERMWIDGET_LIBRARY_NAME terminalwidget5)
set(COLORSCHEMES_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${TERMWIDGET_LIBRARY_NAME}/color-schemes")
message(STATUS "Color schemes will be installed in: ${COLORSCHEMES_DIR}" )

set(TRANSLATIONS_DIR "${CMAKE_INSTALL_FULL_DATADIR}/${TERMWIDGET_LIBRARY_NAME}/translations")
message(STATUS "Translations will be installed in: ${TRANSLATIONS_DIR}")

set(TERMWIDGET_LIBRARY_NAME termwidget5)
set(TERMINALWIDGET_INCLUDE_DIR "${CMAKE_INSTALL_FULL_INCLUDEDIR}/${TERMWIDGET_LIBRARY_NAME}")

CHECK_FUNCTION_EXISTS(updwtmpx HAVE_UPDWTMPX)

qt5_wrap_cpp(MOCS ${HDRS})
qt5_wrap_ui(UI_SRCS ${UI})
set(PKG_CONFIG_REQ "Qt5Widgets")

lxqt_translate_ts(TERMINALWIDGET_QM
    TRANSLATION_DIR "../../3rdparty/terminalwidget/lib/translations"
    UPDATE_TRANSLATIONS
        ${UPDATE_TRANSLATIONS}
    SOURCES
        ${SRCS} ${HDRS} ${UI}
    INSTALL_DIR
        ${TRANSLATIONS_DIR}
    COMPONENT
        Runtime
)

add_library(${TERMWIDGET_LIBRARY_NAME} SHARED ${SRCS} ${MOCS} ${UI_SRCS} ${TERMINALWIDGET_QM})
target_link_libraries(${TERMWIDGET_LIBRARY_NAME} Qt5::Widgets)
if (USE_TEST)
    # set fvisibility to default only in Unit Test mode
    target_compile_options(${TERMWIDGET_LIBRARY_NAME} PUBLIC "-fvisibility=default")
endif()
set_target_properties( ${TERMWIDGET_LIBRARY_NAME} PROPERTIES
                       SOVERSION ${TERMINALWIDGET_VERSION_MAJOR}
                       VERSION ${TERMINALWIDGET_VERSION}
                     )


if(APPLE)
    target_compile_definitions(${TERMWIDGET_LIBRARY_NAME}
        PRIVATE
            "HAVE_UTMPX"
            "UTMPX_COMPAT"
    )
endif()

if(HAVE_UPDWTMPX)
    target_compile_definitions(${TERMWIDGET_LIBRARY_NAME}
        PRIVATE
            "HAVE_UPDWTMPX"
    )
endif()

if (TERMINALWIDGET_USE_UTEMPTER)
    CHECK_INCLUDE_FILE(utempter.h HAVE_UTEMPTER)
    if (HAVE_UTEMPTER)
        target_compile_definitions(${TERMWIDGET_LIBRARY_NAME} PRIVATE
                "HAVE_UTEMPTER"
        )
        target_link_libraries(${TERMWIDGET_LIBRARY_NAME} ulog)
    endif()
endif()

if (UTF8PROC_FOUND)
    target_compile_definitions(${TERMWIDGET_LIBRARY_NAME}
        PRIVATE
            "HAVE_UTF8PROC"
    )
    target_include_directories(${TERMWIDGET_LIBRARY_NAME}
        INTERFACE
            ${UTF8PROC_INCLUDE_DIRS}
    )
    target_link_libraries(${TERMWIDGET_LIBRARY_NAME}
        ${UTF8PROC_LIBRARIES}
    )
    string(APPEND PKG_CONFIG_REQ ", libutf8proc")
endif()

if(APPLE)
    set (CMAKE_SKIP_RPATH 1)
    # this is a must to load the lib correctly
    set_target_properties(${TERMWIDGET_LIBRARY_NAME} PROPERTIES INSTALL_NAME_DIR ${CMAKE_INSTALL_FULL_LIBDIR})
endif()

target_compile_definitions(${TERMWIDGET_LIBRARY_NAME}
    PRIVATE
        "KB_LAYOUT_DIR=\"${KB_LAYOUT_DIR}\""
        "COLORSCHEMES_DIR=\"${COLORSCHEMES_DIR}\""
        "TRANSLATIONS_DIR=\"${TRANSLATIONS_DIR}\""
        "HAVE_POSIX_OPENPT"
        "HAVE_SYS_TIME_H"
)

target_include_directories(${TERMWIDGET_LIBRARY_NAME}
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/lib>"
    INTERFACE
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${TERMWIDGET_LIBRARY_NAME}>"
)

write_basic_package_version_file(
    "${CMAKE_BINARY_DIR}/${TERMWIDGET_LIBRARY_NAME}-config-version.cmake"
    VERSION ${TERMINALWIDGET_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_BINARY_DIR}/${TERMWIDGET_LIBRARY_NAME}-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${TERMWIDGET_LIBRARY_NAME}"
    COMPONENT Devel
)

install(EXPORT
    "${TERMWIDGET_LIBRARY_NAME}-targets"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${TERMWIDGET_LIBRARY_NAME}"
    COMPONENT Devel
)

install(FILES
    ${HDRS_DISTRIB} "${CMAKE_CURRENT_BINARY_DIR}/../../3rdparty/terminalwidget/lib/qtermwidget_export.h"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${TERMWIDGET_LIBRARY_NAME}"
    COMPONENT Devel
)
# keyboard layouts
install(DIRECTORY
    ../../3rdparty/terminalwidget/lib/kb-layouts/
    DESTINATION "${KB_LAYOUT_DIR}"
    COMPONENT Runtime
    FILES_MATCHING PATTERN "*.keytab"
)
# color schemes
install(DIRECTORY
    ../../3rdparty/terminalwidget/lib/color-schemes/
    DESTINATION "${COLORSCHEMES_DIR}"
    COMPONENT Runtime
    FILES_MATCHING PATTERN "*.*schem*"
)

lxqt_create_pkgconfig_file(
    PACKAGE_NAME ${TERMWIDGET_LIBRARY_NAME}
    DESCRIPTIVE_NAME ${TERMWIDGET_LIBRARY_NAME}
    DESCRIPTION "TerminalWidget library for Qt ${TERMINALWIDGET_VERSION_MAJOR}.x"
    INCLUDEDIRS ${TERMWIDGET_LIBRARY_NAME}
    LIBS ${TERMWIDGET_LIBRARY_NAME}
    REQUIRES ${PKG_CONFIG_REQ}
    VERSION ${TERMINALWIDGET_VERSION}
    INSTALL
    COMPONENT Devel
)

#configure_file(
#    "${PROJECT_SOURCE_DIR}/cmake/${TERMWIDGET_LIBRARY_NAME}-config.cmake.in"
#    "${CMAKE_BINARY_DIR}/${TERMWIDGET_LIBRARY_NAME}-config.cmake"
#    @ONLY
#)

install(FILES
    "${CMAKE_BINARY_DIR}/${TERMWIDGET_LIBRARY_NAME}-config.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${TERMWIDGET_LIBRARY_NAME}"
    COMPONENT Devel
)

install(TARGETS ${TERMWIDGET_LIBRARY_NAME}
    DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    EXPORT "${TERMWIDGET_LIBRARY_NAME}-targets"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    PUBLIC_HEADER
    COMPONENT Runtime
)

export(TARGETS ${TERMWIDGET_LIBRARY_NAME}
    FILE "${CMAKE_BINARY_DIR}/${TERMWIDGET_LIBRARY_NAME}-targets.cmake"
    EXPORT_LINK_INTERFACE_LIBRARIES
)
# end of main library


# example application
if(BUILD_EXAMPLE)
    set(EXAMPLE_SRC example/main.cpp)
    add_executable(example ${EXAMPLE_SRC})
    target_link_libraries(example ${TERMWIDGET_LIBRARY_NAME})
endif()
# end of example application

# python binding
if (TERMINALWIDGET_BUILD_PYTHON_BINDING)
    add_subdirectory(pyqt)
endif()
# end of python binding


#CONFIGURE_FILE(
#    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
#    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
#    IMMEDIATE @ONLY
#)
#ADD_CUSTOM_TARGET(uninstall
#    "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
#)
