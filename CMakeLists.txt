cmake_minimum_required(VERSION 3.24)
project(pp-cpp-ui VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PP_UI_BUILD_TESTS "Build pp-cpp-ui / RmlUi unit tests" OFF)

set(PP_UI_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
set(PP_UI_RMLUI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/rmlui")
set(PP_UI_RMLUI_INCLUDE "${PP_UI_RMLUI_ROOT}/Include")
set(PP_UI_BACKEND_DIR "${CMAKE_CURRENT_SOURCE_DIR}/backend")

# Export paths for consumers (pp-browser FontEngineHarfBuzz includes, etc.).
set(PP_LIB_RMLUI_ROOT "${PP_UI_RMLUI_ROOT}" CACHE PATH "RmlUi fork root from pp-cpp-ui" FORCE)
set(PP_LIB_RMLUI_INCLUDE "${PP_UI_RMLUI_INCLUDE}" CACHE PATH "RmlUi Include/ from pp-cpp-ui" FORCE)

# --- Vendored deps (idempotent when parent already defined targets) ---

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/PpUiVendors.cmake")
pp_ui_add_vendored_deps()
pp_ui_add_sdl_deps()

# --- Fixed embedding profile (matches former pp_lib_apply_rmlui_product_profile) ---

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/PpUiProfile.cmake")
pp_ui_apply_rmlui_profile()

if(PP_UI_BUILD_TESTS OR RMLUI_TESTS)
  enable_testing()
endif()

add_subdirectory("${PP_UI_RMLUI_ROOT}"
                 "${CMAKE_CURRENT_BINARY_DIR}/rmlui"
                 EXCLUDE_FROM_ALL)

if(NOT TARGET RmlUi::Core)
  message(FATAL_ERROR "pp-cpp-ui: RmlUi::Core was not defined")
endif()

# --- pp_ui_rml: RmlUi core + include path ---

if(NOT TARGET pp_ui_rml)
  add_library(pp_ui_rml INTERFACE)
  add_library(pp::ui_rml ALIAS pp_ui_rml)
  target_link_libraries(pp_ui_rml INTERFACE RmlUi::Core)
  target_include_directories(pp_ui_rml INTERFACE
    $<BUILD_INTERFACE:${PP_UI_RMLUI_INCLUDE}>
  )
  # MSVC does not reliably resolve PRIVATE static-lib deps (rmlui_core -> lunasvg).
  if(WIN32 AND TARGET lunasvg::lunasvg)
    target_link_libraries(pp_ui_rml INTERFACE lunasvg::lunasvg)
  endif()
endif()

# --- pp_ui_backend: Platform_SDL + Renderer_GL3 ---

if(NOT TARGET pp_ui_backend)
  add_library(pp_ui_backend STATIC
    "${PP_UI_BACKEND_DIR}/RmlUi_Platform_SDL.cpp"
    "${PP_UI_BACKEND_DIR}/RmlUi_Renderer_GL3.cpp"
  )
  add_library(pp::ui_backend ALIAS pp_ui_backend)

  target_include_directories(pp_ui_backend PUBLIC
    $<BUILD_INTERFACE:${PP_UI_BACKEND_DIR}>
  )
  target_compile_definitions(pp_ui_backend PUBLIC RMLUI_SDL_VERSION_MAJOR=3)
  target_link_libraries(pp_ui_backend PUBLIC
    pp_ui_rml
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

# --- pp_ui: umbrella (rml + backend) ---

if(NOT TARGET pp_ui)
  add_library(pp_ui INTERFACE)
  add_library(pp::ui ALIAS pp_ui)
  target_link_libraries(pp_ui INTERFACE pp_ui_rml pp_ui_backend)
endif()

# Helper for consumers that link RmlUi::Core directly (parity with old pp_browser_link_*).
function(pp_ui_link_svg_deps target)
  if(WIN32 AND TARGET lunasvg::lunasvg)
    target_link_libraries(${target} PRIVATE lunasvg::lunasvg)
  endif()
endfunction()
