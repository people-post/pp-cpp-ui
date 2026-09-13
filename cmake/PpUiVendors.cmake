# Add FreeType + HarfBuzz + LunaSVG (+ zlib/libpng when needed) for the pp-cpp-ui fork.

function(pp_ui_add_vendored_deps)
  set(_tp "${PP_UI_THIRD_PARTY_DIR}")

  # FreeType needs zlib+libpng for Noto Color Emoji CBDT bitmaps.
  set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
  set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
  set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
  set(FT_REQUIRE_BROTLI OFF CACHE BOOL "" FORCE)
  set(FT_DISABLE_PNG OFF CACHE BOOL "" FORCE)
  set(FT_REQUIRE_PNG ON CACHE BOOL "" FORCE)
  set(FT_DISABLE_ZLIB OFF CACHE BOOL "" FORCE)
  set(FT_REQUIRE_ZLIB ON CACHE BOOL "" FORCE)

  # Always vendor zlib+libpng. find_package(PNG) on macOS picks Homebrew shared
  # dylibs that Developer ID / notarized apps cannot load (Team ID mismatch).
  set(ZLIB_FOUND FALSE)
  set(PNG_FOUND FALSE)

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

# --- SDL3 + SDL3_image (idempotent when parent already defined targets) ---

function(pp_ui_add_sdl_deps)
  set(_tp "${PP_UI_THIRD_PARTY_DIR}")

  if(NOT TARGET SDL3::SDL3 AND NOT TARGET SDL3::SDL3-static AND NOT TARGET SDL3-static AND NOT TARGET SDL3)
    message(STATUS "pp-cpp-ui: vendoring SDL3 + SDL3_image")

    if(ANDROID OR CMAKE_SYSTEM_NAME STREQUAL "iOS")
      set(SDL_UNIX_CONSOLE_BUILD OFF CACHE BOOL "SDL console build without windowing" FORCE)
      set(SDL_OPENGL OFF CACHE BOOL "Desktop OpenGL/GLX disabled on mobile" FORCE)
      set(SDL_OPENGLES ON CACHE BOOL "OpenGL ES for mobile SDL video" FORCE)
      set(SDL_X11 OFF CACHE BOOL "" FORCE)
    elseif(UNIX AND NOT APPLE)
      if(NOT EXISTS "/usr/include/X11/Xlib.h")
        message(FATAL_ERROR
          "pp-cpp-ui: X11 development headers are required for SDL3.\n"
          "  Debian/Ubuntu: sudo apt install libx11-dev libxext-dev libxcursor-dev "
          "libxinerama-dev libxi-dev libxrandr-dev libxfixes-dev")
      endif()
      if(NOT EXISTS "/usr/include/GL/gl.h")
        message(FATAL_ERROR
          "pp-cpp-ui: OpenGL development headers are required (GL3 backend).\n"
          "  Debian/Ubuntu: sudo apt install libgl-dev")
      endif()
      set(SDL_UNIX_CONSOLE_BUILD OFF CACHE BOOL "SDL console build without windowing" FORCE)
      set(SDL_OPENGL ON CACHE BOOL "Include OpenGL/GLX in SDL3" FORCE)
      set(SDL_OPENGLES OFF CACHE BOOL "Disable OpenGL ES (desktop GL3 backend)" FORCE)
      set(SDL_X11 ON CACHE BOOL "" FORCE)
    endif()

    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_DBUS OFF CACHE BOOL "" FORCE)
    set(SDL_IBUS OFF CACHE BOOL "" FORCE)
    set(SDL_WAYLAND OFF CACHE BOOL "" FORCE)
    set(SDL_AUDIO ON CACHE BOOL "" FORCE)
    set(SDL_RENDER OFF CACHE BOOL "" FORCE)
    set(SDL_GPU OFF CACHE BOOL "" FORCE)
    set(SDL_CAMERA ON CACHE BOOL "" FORCE)
    set(SDL_JOYSTICK OFF CACHE BOOL "" FORCE)
    set(SDL_HAPTIC OFF CACHE BOOL "" FORCE)
    set(SDL_SENSOR OFF CACHE BOOL "" FORCE)
    if(ANDROID)
      set(SDL_HIDAPI ON CACHE BOOL "" FORCE)
    else()
      set(SDL_HIDAPI OFF CACHE BOOL "" FORCE)
    endif()
    set(SDL_DIALOG ON CACHE BOOL "" FORCE)
    set(SDL_VULKAN OFF CACHE BOOL "" FORCE)
    set(SDL_PIPEWIRE OFF CACHE BOOL "" FORCE)
    set(SDL_LIBUDEV OFF CACHE BOOL "" FORCE)
    set(SDL_LIBURING OFF CACHE BOOL "" FORCE)
    if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
      set(SDL_PULSEAUDIO ON CACHE BOOL "" FORCE)
      set(SDL_ALSA ON CACHE BOOL "" FORCE)
    endif()

    add_subdirectory("${_tp}/sdl3"
                     "${CMAKE_CURRENT_BINARY_DIR}/third_party/sdl3" EXCLUDE_FROM_ALL)

    # SDL3_image looks for SDL3::SDL3 (not only SDL3::SDL3-static) before find_package.
    # Alias the real target (not an ALIAS) to avoid CMake alias-of-alias errors.
    if(NOT TARGET SDL3::SDL3)
      if(TARGET SDL3-static)
        add_library(SDL3::SDL3 ALIAS SDL3-static)
      elseif(TARGET SDL3)
        add_library(SDL3::SDL3 ALIAS SDL3)
      endif()
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
      if(TARGET SDL3-static)
        target_link_libraries(SDL3-static PUBLIC "$<LINK_LIBRARY:FRAMEWORK,GameController>")
      elseif(TARGET SDL3)
        target_link_libraries(SDL3 PUBLIC "$<LINK_LIBRARY:FRAMEWORK,GameController>")
      endif()
    endif()

    # Match static SDL; SDL3_image picks SDL3::SDL3-shared when BUILD_SHARED_LIBS is ON.
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

    set(SDLIMAGE_BACKEND_STB ON CACHE BOOL "" FORCE)
    set(SDLIMAGE_BACKEND_WIC OFF CACHE BOOL "" FORCE)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
      set(SDLIMAGE_BACKEND_IMAGEIO OFF CACHE BOOL "" FORCE)
    endif()
    set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_TIF OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_WEBP OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_BMP ON CACHE BOOL "" FORCE)
    set(SDLIMAGE_JPG ON CACHE BOOL "" FORCE)
    set(SDLIMAGE_PNG ON CACHE BOOL "" FORCE)
    set(SDLIMAGE_INSTALL OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_SAMPLES OFF CACHE BOOL "" FORCE)
    set(SDLIMAGE_TESTS OFF CACHE BOOL "" FORCE)

    if(NOT EXISTS "${_tp}/sdl3_image/external/libavif/CMakeLists.txt")
      message(FATAL_ERROR
        "pp-cpp-ui: missing SDL3_image external codecs under third_party/sdl3_image/external/.")
    endif()

    add_subdirectory("${_tp}/sdl3_image"
                     "${CMAKE_CURRENT_BINARY_DIR}/third_party/sdl3_image" EXCLUDE_FROM_ALL)
  else()
    message(STATUS "pp-cpp-ui: using parent-provided SDL3 targets")
  endif()

  if(TARGET SDL3::SDL3-static)
    set(_pp_ui_sdl3 SDL3::SDL3-static)
  elseif(TARGET SDL3::SDL3)
    set(_pp_ui_sdl3 SDL3::SDL3)
  elseif(TARGET SDL3-static)
    set(_pp_ui_sdl3 SDL3-static)
  elseif(TARGET SDL3)
    set(_pp_ui_sdl3 SDL3)
  else()
    message(FATAL_ERROR "pp-cpp-ui: SDL3 target not found")
  endif()

  if(TARGET SDL3_image::SDL3_image-static)
    set(_pp_ui_sdl3_image SDL3_image::SDL3_image-static)
  elseif(TARGET SDL3_image::SDL3_image)
    set(_pp_ui_sdl3_image SDL3_image::SDL3_image)
  else()
    message(FATAL_ERROR "pp-cpp-ui: SDL3_image target not found")
  endif()

  if(NOT TARGET SDL3::SDL3)
    add_library(SDL3::SDL3 ALIAS ${_pp_ui_sdl3})
  endif()
  if(NOT TARGET SDL3_image::SDL3_image)
    add_library(SDL3_image::SDL3_image ALIAS ${_pp_ui_sdl3_image})
  endif()

  # pp-cpp-ui backend find modules expect these version-independent aliases.
  if(NOT TARGET SDL::SDL)
    add_library(SDL_alias INTERFACE)
    add_library(SDL::SDL ALIAS SDL_alias)
    target_link_libraries(SDL_alias INTERFACE ${_pp_ui_sdl3})
    target_compile_definitions(SDL_alias INTERFACE UI_SDL_VERSION_MAJOR=3)
  endif()
  if(NOT TARGET SDL_image::SDL_image)
    add_library(SDL_image_alias INTERFACE)
    add_library(SDL_image::SDL_image ALIAS SDL_image_alias)
    target_link_libraries(SDL_image_alias INTERFACE ${_pp_ui_sdl3_image})
  endif()

  set(PP_UI_SDL3_TARGET "${_pp_ui_sdl3}" CACHE STRING "SDL3 CMake target from pp-cpp-ui" FORCE)
  set(PP_UI_SDL3_IMAGE_TARGET "${_pp_ui_sdl3_image}" CACHE STRING "SDL3_image CMake target from pp-cpp-ui" FORCE)
endfunction()
