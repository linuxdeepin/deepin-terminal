#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "terminalwidget5" for configuration "Debug"
set_property(TARGET terminalwidget5 APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(terminalwidget5 PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/x86_64-linux-gnu/libterminalwidget5.so.0.14.1"
  IMPORTED_SONAME_DEBUG "libterminalwidget5.so.0"
  )

list(APPEND _IMPORT_CHECK_TARGETS terminalwidget5 )
list(APPEND _IMPORT_CHECK_FILES_FOR_terminalwidget5 "${_IMPORT_PREFIX}/lib/x86_64-linux-gnu/libterminalwidget5.so.0.14.1" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
