# Fixed pp-cpp-ui embedding profile for People Post C++ apps.
# Mirrors former pp-browser pp_lib_apply_rmlui_product_profile().

function(pp_ui_apply_profile)
  message(STATUS "pp-cpp-ui: configuring pp-cpp-ui fork")

  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(UI_SAMPLES OFF CACHE BOOL "" FORCE)
  set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
  set(UI_LOTTIE_PLUGIN OFF CACHE BOOL "" FORCE)
  set(UI_SVG_PLUGIN ON CACHE BOOL "" FORCE)
  set(UI_LUA_BINDINGS OFF CACHE BOOL "" FORCE)
  set(UI_FONT_ENGINE_HARFBUZZ ON CACHE BOOL "HarfBuzz text shaping for pp-cpp-ui font engine" FORCE)

  if(PP_UI_BUILD_TESTS)
    set(UI_TESTS ON CACHE BOOL "Build pp-cpp-ui unit tests" FORCE)
    # Force SDL_GL3 so CI/Linux never auto-selects GLFW.
    set(UI_BACKEND SDL_GL3 CACHE STRING "pp-cpp-ui samples/tests backend" FORCE)
    set(UI_SDL_VERSION_MAJOR "3" CACHE STRING "SDL major version for pp-cpp-ui backends" FORCE)
  elseif(NOT UI_TESTS)
    set(UI_TESTS OFF CACHE BOOL "Build pp-cpp-ui unit tests" FORCE)
  endif()
endfunction()
