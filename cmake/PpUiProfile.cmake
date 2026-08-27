# Fixed RmlUi embedding profile for People Post C++ apps.
# Mirrors former pp-browser pp_lib_apply_rmlui_product_profile().

function(pp_ui_apply_rmlui_profile)
  message(STATUS "pp-cpp-ui: configuring RmlUi fork")

  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(RMLUI_SAMPLES OFF CACHE BOOL "" FORCE)
  set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
  set(RMLUI_LOTTIE_PLUGIN OFF CACHE BOOL "" FORCE)
  set(RMLUI_SVG_PLUGIN ON CACHE BOOL "" FORCE)
  set(RMLUI_LUA_BINDINGS OFF CACHE BOOL "" FORCE)
  set(RMLUI_FONT_ENGINE_HARFBUZZ ON CACHE BOOL "HarfBuzz text shaping for RmlUi font engine" FORCE)

  if(PP_UI_BUILD_TESTS)
    set(RMLUI_TESTS ON CACHE BOOL "Build RmlUi unit tests" FORCE)
    # Force SDL_GL3 so CI/Linux never auto-selects GLFW.
    set(RMLUI_BACKEND SDL_GL3 CACHE STRING "RmlUi samples/tests backend" FORCE)
    set(RMLUI_SDL_VERSION_MAJOR "3" CACHE STRING "SDL major version for RmlUi backends" FORCE)
  elseif(NOT RMLUI_TESTS)
    set(RMLUI_TESTS OFF CACHE BOOL "Build RmlUi unit tests" FORCE)
  endif()
endfunction()
