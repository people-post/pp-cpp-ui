# Bootstrap the owned UI engine (formerly the RmlUi subdirectory project).
# Called from the repo root after vendors + product profile are applied.

function(pp_ui_add_engine)
  message(STATUS "pp-cpp-ui: configuring first-party UI engine")

  set(PP_UI_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include" CACHE PATH "pp-cpp-ui public include root" FORCE)

  # Export consumer-facing paths (replaces former rmlui/ root vars).
  set(PP_LIB_RMLUI_ROOT "${CMAKE_SOURCE_DIR}" CACHE PATH "pp-cpp-ui engine root" FORCE)
  set(PP_LIB_RMLUI_INCLUDE "${PP_UI_INCLUDE_DIR}" CACHE PATH "Public headers (include/)" FORCE)

  # Product-fixed engine options (no upstream option matrix).
  set(RMLUI_FONT_ENGINE "freetype" CACHE STRING "Font engine" FORCE)
  set(RMLUI_LOTTIE_PLUGIN OFF CACHE BOOL "" FORCE)
  set(RMLUI_SVG_PLUGIN ON CACHE BOOL "" FORCE)
  set(RMLUI_LUA_BINDINGS OFF CACHE BOOL "" FORCE)
  set(RMLUI_FONT_ENGINE_HARFBUZZ ON CACHE BOOL "" FORCE)
  set(RMLUI_COMPILER_OPTIONS ON CACHE BOOL "" FORCE)
  set(RMLUI_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
  set(RMLUI_PRECOMPILED_HEADERS ON CACHE BOOL "" FORCE)
  set(RMLUI_THIRDPARTY_CONTAINERS ON CACHE BOOL "" FORCE)
  set(RMLUI_CUSTOM_RTTI OFF CACHE BOOL "" FORCE)
  set(RMLUI_MATRIX_ROW_MAJOR OFF CACHE BOOL "" FORCE)
  set(RMLUI_TRACY_PROFILING OFF CACHE BOOL "" FORCE)
  set(RMLUI_CUSTOM_CONFIGURATION OFF CACHE BOOL "" FORCE)
  set(RMLUI_HARFBUZZ_SAMPLE OFF CACHE BOOL "" FORCE)
  set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

  set(RMLUI_VERSION_RELEASE TRUE)
  set(RMLUI_VERSION_SHORT "6.2")

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

  if(NOT TARGET rmlui)
    add_library(rmlui INTERFACE)
    add_library(RmlUi::RmlUi ALIAS rmlui)
    target_link_libraries(rmlui INTERFACE rmlui_core rmlui_debugger)
    set_target_properties(rmlui PROPERTIES EXPORT_NAME "RmlUi")
  endif()

  if(PP_UI_BUILD_TESTS OR RMLUI_TESTS)
    set(RMLUI_SHELL ON)
    set(RMLUI_BACKEND "SDL_GL3" CACHE STRING "" FORCE)
    set(RMLUI_SDL_VERSION_MAJOR "3" CACHE STRING "" FORCE)
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
