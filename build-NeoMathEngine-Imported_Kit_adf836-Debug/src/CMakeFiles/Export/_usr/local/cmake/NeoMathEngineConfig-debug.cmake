#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "NeoML::NeoMathEngine" for configuration "Debug"
set_property(TARGET NeoML::NeoMathEngine APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(NeoML::NeoMathEngine PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libNeoMathEngine.so"
  IMPORTED_SONAME_DEBUG "libNeoMathEngine.so"
  )

list(APPEND _IMPORT_CHECK_TARGETS NeoML::NeoMathEngine )
list(APPEND _IMPORT_CHECK_FILES_FOR_NeoML::NeoMathEngine "${_IMPORT_PREFIX}/lib/libNeoMathEngine.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
