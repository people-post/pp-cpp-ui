# Add FreeType + HarfBuzz + LunaSVG (+ zlib/libpng when needed) for the RmlUi fork.

function(pp_ui_add_vendored_deps)
  set(_tp "${PP_UI_THIRD_PARTY_DIR}")

  # FreeType needs zlib+libpng for Noto Color Emoji CBDT bitmaps.
  set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
  set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
  set(FT_DISABLE_PNG OFF CACHE BOOL "" FORCE)
  set(FT_REQUIRE_PNG ON CACHE BOOL "" FORCE)
  set(FT_DISABLE_ZLIB OFF CACHE BOOL "" FORCE)
  set(FT_REQUIRE_ZLIB ON CACHE BOOL "" FORCE)

  if(WIN32)
    set(ZLIB_FOUND FALSE)
    set(PNG_FOUND FALSE)
  else()
    find_package(ZLIB QUIET)
    find_package(PNG QUIET)
  endif()

  if(NOT ZLIB_FOUND OR NOT PNG_FOUND)
    message(STATUS "pp-cpp-ui: vendoring zlib+libpng for FreeType color emoji")
    unset(ZLIB_LIBRARY CACHE)
    unset(ZLIB_LIBRARY_RELEASE CACHE)
    unset(ZLIB_LIBRARY_DEBUG CACHE)
    unset(PNG_LIBRARY CACHE)
    unset(PNG_LIBRARY_RELEASE CACHE)
    unset(PNG_LIBRARY_DEBUG CACHE)
    unset(PNG_PNG_INCLUDE_DIR CACHE)
    set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    set(_pp_added_zlib FALSE)
    if(NOT TARGET zlibstatic AND NOT TARGET zlib)
      add_subdirectory("${_tp}/zlib"
                       "${CMAKE_CURRENT_BINARY_DIR}/third_party/zlib" EXCLUDE_FROM_ALL)
      set(_pp_added_zlib TRUE)
    endif()
    if(TARGET zlibstatic)
      set(_pp_zlib_lib zlibstatic)
    else()
      set(_pp_zlib_lib zlib)
    endif()
    if(_pp_added_zlib)
      set(_pp_zlib_inc_list
          "${_tp}/zlib"
          "${CMAKE_CURRENT_BINARY_DIR}/third_party/zlib")
    else()
      # Parent already provided zlib (e.g. browser libp2p). Match its headers only.
      get_target_property(_pp_zlib_src ${_pp_zlib_lib} SOURCE_DIR)
      get_target_property(_pp_zlib_bin ${_pp_zlib_lib} BINARY_DIR)
      if(NOT _pp_zlib_src)
        message(FATAL_ERROR "pp-cpp-ui: parent zlib target has no SOURCE_DIR")
      endif()
      set(_pp_zlib_inc_list "${_pp_zlib_src}")
      if(_pp_zlib_bin)
        list(APPEND _pp_zlib_inc_list "${_pp_zlib_bin}")
      endif()
    endif()
    set(ZLIB_LIBRARY ${_pp_zlib_lib} CACHE FILEPATH "zlib for FreeType/libpng" FORCE)
    set(ZLIB_INCLUDE_DIR "${_pp_zlib_inc_list}" CACHE PATH "zlib include for FreeType/libpng" FORCE)
    set(ZLIB_FOUND TRUE CACHE BOOL "" FORCE)
    set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
    set(ZLIB_INCLUDE_DIRS "${_pp_zlib_inc_list}")
    if(NOT TARGET ZLIB::ZLIB)
      add_library(ZLIB::ZLIB ALIAS ${_pp_zlib_lib})
    endif()

    set(PNG_SHARED OFF CACHE BOOL "" FORCE)
    set(PNG_STATIC ON CACHE BOOL "" FORCE)
    set(PNG_TESTS OFF CACHE BOOL "" FORCE)
    set(PNG_TOOLS OFF CACHE BOOL "" FORCE)
    set(PNG_FRAMEWORK OFF CACHE BOOL "" FORCE)
    set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
    if(MSVC AND NOT CMAKE_ASM_COMPILE_OBJECT)
      set(CMAKE_ASM_COMPILE_OBJECT
          "<CMAKE_C_COMPILER> <DEFINES> <INCLUDES> <FLAGS> /c /Fo<OBJECT> <SOURCE>")
    endif()
    if(NOT TARGET png_static AND NOT TARGET png_shared AND NOT TARGET png)
      add_subdirectory("${_tp}/libpng"
                       "${CMAKE_CURRENT_BINARY_DIR}/third_party/libpng" EXCLUDE_FROM_ALL)
    endif()
    if(TARGET png_static)
      set(_pp_png_lib png_static)
    elseif(TARGET png)
      set(_pp_png_lib png)
    else()
      message(FATAL_ERROR "pp-cpp-ui: vendored libpng target not found")
    endif()
    # libpng may find mismatched zlib headers — force the zlib we selected above.
    target_include_directories(${_pp_png_lib} BEFORE PRIVATE ${_pp_zlib_inc_list})
    target_link_libraries(${_pp_png_lib} PUBLIC ${_pp_zlib_lib})

    set(_pp_png_cfg_dir "${CMAKE_CURRENT_BINARY_DIR}/pp_cmake_png")
    file(MAKE_DIRECTORY "${_pp_png_cfg_dir}")
    file(WRITE "${_pp_png_cfg_dir}/PNGConfig.cmake"
         "set(PNG_FOUND TRUE)\n"
         "set(PNG_INCLUDE_DIRS \"${_tp}/libpng\")\n"
         "set(PNG_LIBRARIES ${_pp_png_lib})\n"
         "set(PNG_LIBRARY ${_pp_png_lib})\n"
         "set(PNG_PNG_INCLUDE_DIR \"${_tp}/libpng\")\n"
         "if(NOT TARGET PNG::PNG)\n"
         "  add_library(PNG::PNG INTERFACE IMPORTED)\n"
         "  set_target_properties(PNG::PNG PROPERTIES\n"
         "    INTERFACE_INCLUDE_DIRECTORIES \"${_tp}/libpng\"\n"
         "    INTERFACE_LINK_LIBRARIES ${_pp_png_lib})\n"
         "endif()\n")
    set(_pp_zlib_cfg_dir "${CMAKE_CURRENT_BINARY_DIR}/pp_cmake_zlib")
    file(MAKE_DIRECTORY "${_pp_zlib_cfg_dir}")
    file(WRITE "${_pp_zlib_cfg_dir}/ZLIBConfig.cmake"
         "set(ZLIB_FOUND TRUE)\n"
         "set(ZLIB_INCLUDE_DIRS \"${ZLIB_INCLUDE_DIR}\")\n"
         "set(ZLIB_LIBRARIES ${_pp_zlib_lib})\n"
         "set(ZLIB_LIBRARY ${_pp_zlib_lib})\n"
         "set(ZLIB_INCLUDE_DIR \"${ZLIB_INCLUDE_DIR}\")\n"
         "if(NOT TARGET ZLIB::ZLIB)\n"
         "  add_library(ZLIB::ZLIB INTERFACE IMPORTED)\n"
         "  set_target_properties(ZLIB::ZLIB PROPERTIES\n"
         "    INTERFACE_INCLUDE_DIRECTORIES \"${ZLIB_INCLUDE_DIR}\"\n"
         "    INTERFACE_LINK_LIBRARIES ${_pp_zlib_lib})\n"
         "endif()\n")
    list(PREPEND CMAKE_PREFIX_PATH "${_pp_png_cfg_dir}" "${_pp_zlib_cfg_dir}")
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)
    set(PNG_DIR "${_pp_png_cfg_dir}" CACHE PATH "Vendored PNG config for FreeType" FORCE)
    set(ZLIB_DIR "${_pp_zlib_cfg_dir}" CACHE PATH "Vendored ZLIB config for FreeType" FORCE)
    set(PNG_FOUND TRUE)
    set(PNG_LIBRARIES ${_pp_png_lib})
    set(PNG_INCLUDE_DIRS "${_tp}/libpng")
  endif()

  if(NOT TARGET Freetype::Freetype AND NOT TARGET freetype AND NOT TARGET freetype-interface)
    add_subdirectory("${_tp}/freetype"
                     "${CMAKE_CURRENT_BINARY_DIR}/third_party/freetype" EXCLUDE_FROM_ALL)
  endif()
  if(NOT TARGET Freetype::Freetype)
    if(TARGET freetype-interface)
      add_library(Freetype::Freetype ALIAS freetype-interface)
    elseif(TARGET freetype)
      add_library(Freetype::Freetype ALIAS freetype)
    else()
      message(FATAL_ERROR "pp-cpp-ui: FreeType target not found")
    endif()
  endif()

  if(NOT TARGET harfbuzz AND NOT TARGET harfbuzz::harfbuzz)
    set(HB_BUILD_UTILS OFF CACHE BOOL "" FORCE)
    set(HB_BUILD_SUBSET OFF CACHE BOOL "" FORCE)
    set(HB_HAVE_GLIB OFF CACHE BOOL "" FORCE)
    set(HB_HAVE_ICU OFF CACHE BOOL "" FORCE)
    set(HB_HAVE_GRAPHITE2 OFF CACHE BOOL "" FORCE)
    set(HB_HAVE_GOBJECT OFF CACHE BOOL "" FORCE)
    # On iOS, HarfBuzz CoreText needs HB_IOS (CoreText/CoreGraphics, not AppServices).
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS" OR PP_BROWSER_IS_IOS)
      set(HB_IOS ON CACHE BOOL "" FORCE)
    endif()
    add_subdirectory("${_tp}/harfbuzz"
                     "${CMAKE_CURRENT_BINARY_DIR}/third_party/harfbuzz" EXCLUDE_FROM_ALL)
  endif()
  if(TARGET harfbuzz AND NOT TARGET harfbuzz::harfbuzz)
    add_library(harfbuzz::harfbuzz ALIAS harfbuzz)
  endif()

  if(NOT TARGET lunasvg AND NOT TARGET lunasvg::lunasvg)
    set(LUNASVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(PLUTOVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory("${_tp}/lunasvg"
                     "${CMAKE_CURRENT_BINARY_DIR}/third_party/lunasvg" EXCLUDE_FROM_ALL)
  endif()
endfunction()
