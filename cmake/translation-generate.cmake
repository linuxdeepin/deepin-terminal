function(TRANSLATION_GENERATE QMS)
  find_package(Qt${QT_VERSION_MAJOR}LinguistTools QUIET)

  if (NOT Qt${QT_VERSION_MAJOR}_LRELEASE_EXECUTABLE)
    set(QT_LRELEASE "/lib/qt${QT_VERSION_MAJOR}/bin/lrelease")
    message(STATUS "NOT found lrelease, set QT_LRELEASE = ${QT_LRELEASE}")
  else()
    set(QT_LRELEASE "${Qt${QT_VERSION_MAJOR}_LRELEASE_EXECUTABLE}")
  endif()

  if(NOT ARGN)
    message(SEND_ERROR "Error: TRANSLATION_GENERATE() called without any .ts path")
    return()
  endif()

  # # 获取 translations 目录下的所有 .ts 文件
  # file(GLOB_RECURSE TS_FILES "${ARGN}/*.ts")
  # 获取指定目录下的所有 .ts 文件，不包括子目录
  file(GLOB TS_FILES "${ARGN}/*.ts")

  set(${QMS})
  foreach(TSFIL ${TS_FILES})
      get_filename_component(FIL_WE ${TSFIL} NAME_WE)
#      get_filename_component(TS_DIR ${TSFIL} DIRECTORY)
      set(QMFIL ${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.qm)
      list(APPEND ${QMS} ${QMFIL})
      add_custom_command(
          OUTPUT ${QMFIL}
          # COMMAND ${Qt5_LUPDATE_EXECUTABLE} ${CMAKE_SOURCE_DIR} -ts ${TSFIL}
          COMMAND ${QT_LRELEASE} ${TSFIL} -qm ${QMFIL}
          DEPENDS ${TSFIL}
          COMMENT "Running ${QT_LRELEASE} on ${TSFIL}"
          VERBATIM
      )
  endforeach()

  set_source_files_properties(${${QMS}} PROPERTIES GENERATED TRUE)
  set(${QMS} ${${QMS}} PARENT_SCOPE)
endfunction()
