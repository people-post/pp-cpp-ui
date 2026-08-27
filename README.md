# pp-cpp-ui

Shared C++ UI stack for People Post apps: hard-forked [RmlUi](https://github.com/mikke89/RmlUi), FreeType / HarfBuzz / LunaSVG, and an SDL3 + OpenGL3 backend.

## Contents

- `rmlui/` — owned RmlUi 6.2 hard fork (selectable text, UA sheet, list markers, …)
- `backend/` — reusable `SystemInterface_SDL` + `RenderInterface_GL3`
- `third_party/` — freetype, harfbuzz, lunasvg, zlib, libpng, **sdl3**, **sdl3_image**

### CMake targets

| Target | Alias | Role |
|--------|-------|------|
| `pp_ui_rml` | `pp::ui_rml` | INTERFACE → `RmlUi::Core` |
| `pp_ui_backend` | `pp::ui_backend` | STATIC SDL/GL3 platform + renderer |
| `pp_ui` | `pp::ui` | INTERFACE umbrella (`rml` + `backend`) |

Also provides vendored `Freetype::Freetype`, `harfbuzz::harfbuzz`, `lunasvg::lunasvg`, `SDL3::*`, `SDL3_image::*` (skipped when a parent already defined those targets).

**Not included** (stay in the app): product window host (`Backend::*` / BrowserHost), mobile GL lifecycle, call video tiles, text loupe, touch-sim overlay, ShellHost/presenters, themes/views, fonts, emoji catalog/picker.

## Build

```bash
cmake -S . -B build -DPP_UI_BUILD_TESTS=ON
cmake --build build --target pp_ui_backend rmlui_unit_tests
ctest --test-dir build --output-on-failure
```

With tests on, RmlUi uses the **SDL_GL3** reference backend (no GLFW). Unit tests default to a dummy render interface and do not open a window.

Linux needs X11 + OpenGL (+ Pulse/ALSA recommended for SDL audio drivers).

## Consume (FetchContent)

Pin a **release tag cut from `main`**:

```cmake
include(FetchContent)
FetchContent_Declare(
  pp_cpp_ui
  GIT_REPOSITORY https://github.com/people-post/pp-cpp-ui.git
  GIT_TAG v0.2.0
)
FetchContent_MakeAvailable(pp_cpp_ui)
target_link_libraries(your_target PUBLIC pp_ui)
# Paths: PP_LIB_RMLUI_ROOT / PP_LIB_RMLUI_INCLUDE
# SDL: PP_UI_SDL3_TARGET / PP_UI_SDL3_IMAGE_TARGET
```

For a local sibling checkout (`../pp-cpp-ui`), pass `-DPP_CPP_UI_SOURCE_DIR=...` or rely on auto-detect from pp-browser.

Release flow: land on `develop` → merge to `main` → tag `vX.Y.Z` on `main`.
