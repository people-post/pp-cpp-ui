cmake_minimum_required(VERSION 3.24)
project(pp-cpp-ui VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PP_UI_BUILD_TESTS "Build pp-cpp-ui / RmlUi unit tests" OFF)

set(PP_UI_THIRD_PARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
set(PP_UI_RMLUI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/rmlui")
set(PP_UI_RMLUI_INCLUDE "${PP_UI_RMLUI_ROOT}/Include")

# Export paths for consumers (pp-browser FontEngineHarfBuzz includes, etc.).
set(PP_LIB_RMLUI_ROOT "${PP_UI_RMLUI_ROOT}" CACHE PATH "RmlUi fork root from pp-cpp-ui" FORCE)
set(PP_LIB_RMLUI_INCLUDE "${PP_UI_RMLUI_INCLUDE}" CACHE PATH "RmlUi Include/ from pp-cpp-ui" FORCE)

# --- Vendored deps (idempotent when parent already defined targets) ---

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/PpUiVendors.cmake")
pp_ui_add_vendored_deps()

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

# Convenience target for consumers.
if(NOT TARGET pp_ui)
  add_library(pp_ui INTERFACE)
  add_library(pp::ui ALIAS pp_ui)
  target_link_libraries(pp_ui INTERFACE RmlUi::Core)
  target_include_directories(pp_ui INTERFACE
    $<BUILD_INTERFACE:${PP_UI_RMLUI_INCLUDE}>
  )
  # MSVC does not reliably resolve PRIVATE static-lib deps (rmlui_core -> lunasvg).
  if(WIN32 AND TARGET lunasvg::lunasvg)
    target_link_libraries(pp_ui INTERFACE lunasvg::lunasvg)
  endif()
endif()

# Helper for consumers that link RmlUi::Core directly (parity with old pp_browser_link_*).
function(pp_ui_link_svg_deps target)
  if(WIN32 AND TARGET lunasvg::lunasvg)
    target_link_libraries(${target} PRIVATE lunasvg::lunasvg)
  endif()
endfunction()
