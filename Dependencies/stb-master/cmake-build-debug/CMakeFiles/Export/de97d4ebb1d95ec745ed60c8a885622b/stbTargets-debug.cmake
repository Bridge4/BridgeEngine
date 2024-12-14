#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "stb::image" for configuration "Debug"
set_property(TARGET stb::image APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(stb::image PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libstb-image.a"
  )

list(APPEND _cmake_import_check_targets stb::image )
list(APPEND _cmake_import_check_files_for_stb::image "${_IMPORT_PREFIX}/lib/libstb-image.a" )

# Import target "stb::image-write" for configuration "Debug"
set_property(TARGET stb::image-write APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(stb::image-write PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libstb-image-write.a"
  )

list(APPEND _cmake_import_check_targets stb::image-write )
list(APPEND _cmake_import_check_files_for_stb::image-write "${_IMPORT_PREFIX}/lib/libstb-image-write.a" )

# Import target "stb::image-resize" for configuration "Debug"
set_property(TARGET stb::image-resize APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(stb::image-resize PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libstb-image-resize.a"
  )

list(APPEND _cmake_import_check_targets stb::image-resize )
list(APPEND _cmake_import_check_files_for_stb::image-resize "${_IMPORT_PREFIX}/lib/libstb-image-resize.a" )

# Import target "stb::perlin" for configuration "Debug"
set_property(TARGET stb::perlin APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(stb::perlin PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libstb-perlin.a"
  )

list(APPEND _cmake_import_check_targets stb::perlin )
list(APPEND _cmake_import_check_files_for_stb::perlin "${_IMPORT_PREFIX}/lib/libstb-perlin.a" )

# Import target "stb::vorbis" for configuration "Debug"
set_property(TARGET stb::vorbis APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(stb::vorbis PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libstb-vorbis.a"
  )

list(APPEND _cmake_import_check_targets stb::vorbis )
list(APPEND _cmake_import_check_files_for_stb::vorbis "${_IMPORT_PREFIX}/lib/libstb-vorbis.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
