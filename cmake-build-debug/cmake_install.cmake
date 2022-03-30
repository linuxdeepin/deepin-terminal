# Install script for directory: /home/maomao/deepin/pr/deepin-terminal

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

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/share/applications/deepin-terminal.desktop")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/share/applications" TYPE FILE FILES "/home/maomao/deepin/pr/deepin-terminal/src/deepin-terminal.desktop")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/share/icons/hicolor/scalable/apps/deepin-terminal.svg")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/share/icons/hicolor/scalable/apps" TYPE FILE FILES "/home/maomao/deepin/pr/deepin-terminal/src/assets/logo/deepin-terminal.svg")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/bin/deepin-terminal")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/bin" TYPE FILE PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE FILES "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/share/deepin-terminal/translations/deepin-terminal.qm;/usr/share/deepin-terminal/translations/deepin-terminal_az.qm;/usr/share/deepin-terminal/translations/deepin-terminal_bo.qm;/usr/share/deepin-terminal/translations/deepin-terminal_br.qm;/usr/share/deepin-terminal/translations/deepin-terminal_ca.qm;/usr/share/deepin-terminal/translations/deepin-terminal_cs.qm;/usr/share/deepin-terminal/translations/deepin-terminal_de.qm;/usr/share/deepin-terminal/translations/deepin-terminal_el.qm;/usr/share/deepin-terminal/translations/deepin-terminal_en.qm;/usr/share/deepin-terminal/translations/deepin-terminal_es.qm;/usr/share/deepin-terminal/translations/deepin-terminal_fi.qm;/usr/share/deepin-terminal/translations/deepin-terminal_fr.qm;/usr/share/deepin-terminal/translations/deepin-terminal_gl_ES.qm;/usr/share/deepin-terminal/translations/deepin-terminal_hr.qm;/usr/share/deepin-terminal/translations/deepin-terminal_hu.qm;/usr/share/deepin-terminal/translations/deepin-terminal_it.qm;/usr/share/deepin-terminal/translations/deepin-terminal_ko.qm;/usr/share/deepin-terminal/translations/deepin-terminal_nl.qm;/usr/share/deepin-terminal/translations/deepin-terminal_pl.qm;/usr/share/deepin-terminal/translations/deepin-terminal_pt.qm;/usr/share/deepin-terminal/translations/deepin-terminal_pt_BR.qm;/usr/share/deepin-terminal/translations/deepin-terminal_ro.qm;/usr/share/deepin-terminal/translations/deepin-terminal_ru.qm;/usr/share/deepin-terminal/translations/deepin-terminal_sq.qm;/usr/share/deepin-terminal/translations/deepin-terminal_sr.qm;/usr/share/deepin-terminal/translations/deepin-terminal_tr.qm;/usr/share/deepin-terminal/translations/deepin-terminal_ug.qm;/usr/share/deepin-terminal/translations/deepin-terminal_uk.qm;/usr/share/deepin-terminal/translations/deepin-terminal_zh_CN.qm;/usr/share/deepin-terminal/translations/deepin-terminal_zh_HK.qm;/usr/share/deepin-terminal/translations/deepin-terminal_zh_TW.qm")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/share/deepin-terminal/translations" TYPE FILE FILES
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_az.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_bo.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_br.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_ca.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_cs.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_de.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_el.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_en.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_es.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_fi.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_fr.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_gl_ES.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_hr.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_hu.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_it.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_ko.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_nl.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_pl.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_pt.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_pt_BR.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_ro.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_ru.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_sq.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_sr.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_tr.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_ug.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_uk.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_zh_CN.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_zh_HK.qm"
    "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/deepin-terminal_zh_TW.qm"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/share/deepin-manual/manual-assets/application//deepin-terminal")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/share/deepin-manual/manual-assets/application/" TYPE DIRECTORY FILES "/home/maomao/deepin/pr/deepin-terminal/src/assets/deepin-terminal")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/3rdparty/terminalwidget/cmake_install.cmake")
  include("/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/tests/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/maomao/deepin/pr/deepin-terminal/cmake-build-debug/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
