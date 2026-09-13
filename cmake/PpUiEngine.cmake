# Bootstrap the owned UI engine (formerly the pp-cpp-ui subdirectory project).
# Called from the repo root after vendors + product profile are applied.

function(pp_ui_add_engine)
  message(STATUS "pp-cpp-ui: configuring first-party UI engine")

  set(PP_UI_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include" CACHE PATH "pp-cpp-ui public include root" FORCE)

  # Export consumer-facing paths (replaces former rmlui/ root vars).
  set(PP_LIB_UI_ROOT "${CMAKE_SOURCE_DIR}" CACHE PATH "pp-cpp-ui engine root" FORCE)
  set(PP_LIB_UI_INCLUDE "${PP_UI_INCLUDE_DIR}" CACHE PATH "Public headers (include/)" FORCE)

  # Product-fixed engine options (no upstream option matrix).
  set(UI_FONT_ENGINE "freetype" CACHE STRING "Font engine" FORCE)
  set(UI_LOTTIE_PLUGIN OFF CACHE BOOL "" FORCE)
  set(UI_SVG_PLUGIN ON CACHE BOOL "" FORCE)
  set(UI_LUA_BINDINGS OFF CACHE BOOL "" FORCE)
  set(UI_FONT_ENGINE_HARFBUZZ ON CACHE BOOL "" FORCE)
  set(UI_COMPILER_OPTIONS ON CACHE BOOL "" FORCE)
  set(UI_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
  set(UI_PRECOMPILED_HEADERS ON CACHE BOOL "" FORCE)
  set(UI_THIRDPARTY_CONTAINERS ON CACHE BOOL "" FORCE)
  set(UI_CUSTOM_RTTI OFF CACHE BOOL "" FORCE)
  set(UI_MATRIX_ROW_MAJOR OFF CACHE BOOL "" FORCE)
  set(UI_TRACY_PROFILING OFF CACHE BOOL "" FORCE)
  set(UI_CUSTOM_CONFIGURATION OFF CACHE BOOL "" FORCE)
  set(UI_HARFBUZZ_SAMPLE OFF CACHE BOOL "" FORCE)
  set(UI_SAMPLES OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

  set(UI_VERSION_RELEASE TRUE)
  set(UI_VERSION_SHORT "6.2")

  list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/engine/Modules")

  include(GNUInstallDirs)
  include("${CMAKE_SOURCE_DIR}/cmake/engine/Utilities.cmake")
  include("${CMAKE_SOURCE_DIR}/cmake/engine/RuntimeUtilities.cmake")
  setup_binary_output_directories()
  setup_runtime_dependency_set_arg()

  foreach(_t IN ITEMS Freetype::Freetype lunasvg::lunasvg harfbuzz::harfbuzz)
    if(NOT TARGET ${_t})
      message(FATAL_ERROR "pp-cpp-ui: missing required target ${_t} (vendors)")
    endif()
  endforeach()
  message(STATUS "Found Freetype::Freetype - FreeType font engine enabled")
  message(STATUS "Found lunasvg::lunasvg - SVG plugin enabled")
  message(STATUS "Found harfbuzz::harfbuzz - HarfBuzz text shaping enabled")

  add_subdirectory("${CMAKE_SOURCE_DIR}/src" "${CMAKE_BINARY_DIR}/src")

  if(NOT TARGET ui_engine)
    add_library(ui_engine INTERFACE)
    add_library(ui::engine ALIAS ui_engine)
    target_link_libraries(ui_engine INTERFACE ui_core ui_debugger)
    set_target_properties(ui_engine PROPERTIES EXPORT_NAME "ui")
  endif()

  if(PP_UI_BUILD_TESTS OR UI_TESTS)
    set(UI_SHELL ON)
    set(UI_BACKEND "SDL_GL3" CACHE STRING "" FORCE)
    set(UI_SDL_VERSION_MAJOR "3" CACHE STRING "" FORCE)
    include("${CMAKE_SOURCE_DIR}/cmake/engine/BackendAutoSelection.cmake")
    include("${CMAKE_SOURCE_DIR}/cmake/engine/DependenciesForBackends.cmake")
    include("${CMAKE_SOURCE_DIR}/cmake/engine/DependenciesForShell.cmake")
    add_subdirectory(
      "${CMAKE_SOURCE_DIR}/tests/support/reference_backends/all"
      "${CMAKE_BINARY_DIR}/tests/reference_backends")
    add_subdirectory(
      "${CMAKE_SOURCE_DIR}/tests/support/shell"
      "${CMAKE_BINARY_DIR}/tests/shell")
    add_subdirectory(
      "${CMAKE_SOURCE_DIR}/tests/engine"
      "${CMAKE_BINARY_DIR}/tests/engine")
  endif()
endfunction()
