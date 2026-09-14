cmake_minimum_required(VERSION 3.24)
project(pp-cpp-ui VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PP_UI_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

option(PP_UI_BUILD_TESTS "Build pp-cpp-ui / pp-cpp-ui unit tests" OFF)

set(PP_UI_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
set(PP_UI_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Consumer-facing paths (replaces former rmlui/Include export).
set(PP_LIB_UI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "pp-cpp-ui engine root" FORCE)
set(PP_LIB_UI_INCLUDE "${PP_UI_INCLUDE_DIR}" CACHE PATH "Public headers (include/)" FORCE)

# --- Vendored deps (idempotent when parent already defined targets) ---

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/PpUiVendors.cmake")
pp_ui_add_vendored_deps()
pp_ui_add_sdl_deps()

# --- Fixed embedding profile ---

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/PpUiProfile.cmake")
pp_ui_apply_profile()

if(PP_UI_BUILD_TESTS OR UI_TESTS)
  enable_testing()
endif()

# --- First-party engine (core / svg / debugger) ---

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/PpUiEngine.cmake")
pp_ui_add_engine()

if(NOT TARGET ui::core)
  message(FATAL_ERROR "pp-cpp-ui: ui::core was not defined")
endif()

# --- pp_ui_core: pp-cpp-ui core + include path ---

if(NOT TARGET pp_ui_core)
  add_library(pp_ui_core INTERFACE)
  add_library(pp::ui_core ALIAS pp_ui_core)
  target_link_libraries(pp_ui_core INTERFACE ui::core)
  target_include_directories(pp_ui_core INTERFACE
    $<BUILD_INTERFACE:${PP_UI_INCLUDE_DIR}>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/base/containers>
  )
  # MSVC does not reliably resolve PRIVATE static-lib deps (ui_core -> lunasvg).
  if(WIN32 AND TARGET lunasvg::lunasvg)
    target_link_libraries(pp_ui_core INTERFACE lunasvg::lunasvg)
  endif()
endif()

# --- pp_ui_backend: Platform_SDL + Renderer_GL3 ---

if(NOT TARGET pp_ui_backend)
  add_library(pp_ui_backend STATIC
    src/platform/Platform_SDL.cpp
    src/render/Renderer_GL3.cpp
    src/render/gl/glad.cpp
  )
  add_library(pp::ui_backend ALIAS pp_ui_backend)

  # Public: include/ (ui/...).
  target_include_directories(pp_ui_backend PUBLIC
    $<BUILD_INTERFACE:${PP_UI_INCLUDE_DIR}>
  )
  target_compile_definitions(pp_ui_backend PUBLIC UI_SDL_VERSION_MAJOR=3)
  target_link_libraries(pp_ui_backend PUBLIC
    pp_ui_core
    ${PP_UI_SDL3_TARGET}
    ${PP_UI_SDL3_IMAGE_TARGET}
  )

  if(UNIX AND NOT APPLE)
    target_link_libraries(pp_ui_backend PUBLIC ${CMAKE_DL_LIBS})
  endif()
  if(ANDROID)
    target_link_libraries(pp_ui_backend PUBLIC log GLESv3 EGL)
  endif()
  if(CMAKE_SYSTEM_NAME STREQUAL "iOS" OR PP_BROWSER_IS_IOS)
    target_compile_definitions(pp_ui_backend PRIVATE GLES_SILENCE_DEPRECATION=1)
  endif()
endif()

# --- pp_ui: umbrella (core + backend) ---

if(NOT TARGET pp_ui)
  add_library(pp_ui INTERFACE)
  add_library(pp::ui ALIAS pp_ui)
  target_link_libraries(pp_ui INTERFACE pp_ui_core pp_ui_backend)
endif()

# Helper for consumers that link ui::core directly (parity with old pp_browser_link_*).
function(pp_ui_link_svg_deps target)
  if(WIN32 AND TARGET lunasvg::lunasvg)
    target_link_libraries(${target} PRIVATE lunasvg::lunasvg)
  endif()
endfunction()
