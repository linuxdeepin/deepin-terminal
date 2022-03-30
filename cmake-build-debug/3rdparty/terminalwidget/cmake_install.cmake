# Install script for directory: /home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xRuntimex" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/share/terminalwidget5/translations/terminalwidget_arn.qm;/usr/share/terminalwidget5/translations/terminalwidget_ast.qm;/usr/share/terminalwidget5/translations/terminalwidget_az.qm;/usr/share/terminalwidget5/translations/terminalwidget_bo.qm;/usr/share/terminalwidget5/translations/terminalwidget_br.qm;/usr/share/terminalwidget5/translations/terminalwidget_ca.qm;/usr/share/terminalwidget5/translations/terminalwidget_cs.qm;/usr/share/terminalwidget5/translations/terminalwidget_cy.qm;/usr/share/terminalwidget5/translations/terminalwidget_da.qm;/usr/share/terminalwidget5/translations/terminalwidget_de.qm;/usr/share/terminalwidget5/translations/terminalwidget_el.qm;/usr/share/terminalwidget5/translations/terminalwidget_en.qm;/usr/share/terminalwidget5/translations/terminalwidget_es.qm;/usr/share/terminalwidget5/translations/terminalwidget_fi.qm;/usr/share/terminalwidget5/translations/terminalwidget_fr.qm;/usr/share/terminalwidget5/translations/terminalwidget_gl.qm;/usr/share/terminalwidget5/translations/terminalwidget_gl_ES.qm;/usr/share/terminalwidget5/translations/terminalwidget_he.qm;/usr/share/terminalwidget5/translations/terminalwidget_hu.qm;/usr/share/terminalwidget5/translations/terminalwidget_it.qm;/usr/share/terminalwidget5/translations/terminalwidget_ja.qm;/usr/share/terminalwidget5/translations/terminalwidget_ko.qm;/usr/share/terminalwidget5/translations/terminalwidget_lt.qm;/usr/share/terminalwidget5/translations/terminalwidget_nb_NO.qm;/usr/share/terminalwidget5/translations/terminalwidget_nl.qm;/usr/share/terminalwidget5/translations/terminalwidget_pl.qm;/usr/share/terminalwidget5/translations/terminalwidget_pt.qm;/usr/share/terminalwidget5/translations/terminalwidget_pt_BR.qm;/usr/share/terminalwidget5/translations/terminalwidget_ru.qm;/usr/share/terminalwidget5/translations/terminalwidget_sq.qm;/usr/share/terminalwidget5/translations/terminalwidget_sr.qm;/usr/share/terminalwidget5/translations/terminalwidget_tr.qm;/usr/share/terminalwidget5/translations/terminalwidget_ug.qm;/usr/share/terminalwidget5/translations/terminalwidget_uk.qm;/usr/share/terminalwidget5/translations/terminalwidget_vi.qm;/usr/share/terminalwidget5/translations/terminalwidget_zh_CN.qm;/usr/share/terminalwidget5/translations/terminalwidget_zh_HK.qm;/usr/share/terminalwidget5/translations/terminalwidget_zh_TW.qm")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/share/terminalwidget5/translations" TYPE FILE FILES
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_arn.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_ast.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_az.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_bo.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_br.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_ca.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_cs.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_cy.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_da.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_de.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_el.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_en.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_es.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_fi.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_fr.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_gl.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_gl_ES.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_he.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_hu.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_it.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_ja.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_ko.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_lt.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_nb_NO.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_nl.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_pl.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_pt.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_pt_BR.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_ru.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_sq.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_sr.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_tr.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_ug.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_uk.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_vi.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_zh_CN.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_zh_HK.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget_zh_TW.qm"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/cmake/terminalwidget5" TYPE FILE FILES "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/terminalwidget5-config-version.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/cmake/terminalwidget5/terminalwidget5-targets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/cmake/terminalwidget5/terminalwidget5-targets.cmake"
         "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/CMakeFiles/Export/lib/x86_64-linux-gnu/cmake/terminalwidget5/terminalwidget5-targets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/cmake/terminalwidget5/terminalwidget5-targets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/cmake/terminalwidget5/terminalwidget5-targets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/cmake/terminalwidget5" TYPE FILE FILES "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/CMakeFiles/Export/lib/x86_64-linux-gnu/cmake/terminalwidget5/terminalwidget5-targets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/cmake/terminalwidget5" TYPE FILE FILES "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/CMakeFiles/Export/lib/x86_64-linux-gnu/cmake/terminalwidget5/terminalwidget5-targets-debug.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/terminalwidget5" TYPE FILE FILES
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/qtermwidget.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/Emulation.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/Filter.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/Session.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/SessionManager.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/history/HistoryFile.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/history/HistoryScroll.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/history/HistoryScrollFile.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/history/HistoryScrollNone.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/history/HistoryType.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/history/HistoryTypeFile.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/history/HistoryTypeNone.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/history/compact/CompactHistoryBlock.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/history/compact/CompactHistoryBlockList.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/history/compact/CompactHistoryLine.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/history/compact/CompactHistoryScroll.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/history/compact/CompactHistoryType.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/BlockArray.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/Character.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/CharacterColor.h"
    "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/TerminalDisplay.h"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/lib/qtermwidget_export.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xRuntimex" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/share/terminalwidget5/kb-layouts/")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/share/terminalwidget5/kb-layouts" TYPE DIRECTORY FILES "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/kb-layouts/" FILES_MATCHING REGEX "/[^/]*\\.keytab$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xRuntimex" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/share/terminalwidget5/color-schemes/")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/share/terminalwidget5/color-schemes" TYPE DIRECTORY FILES "/home/maomao/deepin/pr/deepin-terminal/3rdparty/terminalwidget/lib/color-schemes/" FILES_MATCHING REGEX "/[^/]*\\.[^/]*schem[^/]*$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/pkgconfig" TYPE FILE FILES "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/terminalwidget5.pc")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu" TYPE SHARED_LIBRARY FILES
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/libterminalwidget5.so.0.14.1"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/libterminalwidget5.so.0"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/libterminalwidget5.so.0.14.1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/libterminalwidget5.so.0"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu" TYPE SHARED_LIBRARY FILES "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/libterminalwidget5.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/libterminalwidget5.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/libterminalwidget5.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/libterminalwidget5.so")
    endif()
  endif()
endif()

