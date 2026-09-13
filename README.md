# pp-cpp-ui

Shared C++ UI stack for People Post apps: first-party UI engine (pp-cpp-ui-derived),
FreeType / HarfBuzz / LunaSVG, and an SDL3 + OpenGL3 backend.

## Layout

See [docs/SRC_LAYOUT.md](docs/SRC_LAYOUT.md) and [docs/ADR_001_FIRST_PARTY_LAYOUT.md](docs/ADR_001_FIRST_PARTY_LAYOUT.md).

```text
include/ui/        Engine + backend public API (`pp-cpp-ui/` shims kept)
src/core|svg|debugger|platform|render
third_party/       freetype, harfbuzz, lunasvg, zlib, libpng, sdl3, sdl3_image
tests/             Engine unit tests + support harness
```

### CMake targets

| Target | Alias | Role |
|--------|-------|------|
| `ui_core` | `ui::Core` | UI engine core (+ SVG / HarfBuzz) |
| `ui_debugger` | `ui::Debugger` | Debugger |
| `pp_ui_core` | `pp::ui_core` | INTERFACE → core + `include/` |
| `pp_ui_backend` | `pp::ui_backend` | STATIC SDL/GL3 platform + renderer |
| `pp_ui` | `pp::ui` | INTERFACE umbrella (`rml` + `backend`) |

Also provides vendored `Freetype::Freetype`, `harfbuzz::harfbuzz`, `lunasvg::lunasvg`,
`SDL3::*`, `SDL3_image::*` (skipped when a parent already defined those targets).

**Not included** (stay in the app): product window host, mobile GL lifecycle, call video
tiles, text loupe, touch-sim overlay, ShellHost/presenters, themes/views, fonts, emoji.

## Build

```bash
cmake -S . -B build -DPP_UI_BUILD_TESTS=ON
cmake --build build --target pp_ui_backend ui_unit_tests
ctest --test-dir build --output-on-failure
```

With tests on, the engine unit suite uses the SDL_GL3 reference harness (not the product
backend). Unit tests default to a dummy render interface and do not open a window.

Linux needs X11 + OpenGL (+ Pulse/ALSA recommended for SDL audio drivers).

## Consume (FetchContent)

Pin a **release tag cut from `main`**:

```cmake
include(FetchContent)
FetchContent_Declare(
  pp_cpp_ui
  GIT_REPOSITORY https://github.com/people-post/pp-cpp-ui.git
  GIT_TAG v0.3.0
)
FetchContent_MakeAvailable(pp_cpp_ui)
target_link_libraries(your_target PUBLIC pp_ui)
# Paths: PP_LIB_UI_ROOT / PP_LIB_UI_INCLUDE
# SDL: PP_UI_SDL3_TARGET / PP_UI_SDL3_IMAGE_TARGET
```

For a local sibling checkout (`../pp-cpp-ui`), pass `-DPP_CPP_UI_SOURCE_DIR=...` or rely on
auto-detect from pp-browser.

Release flow: land on `develop` → merge to `main` → tag `vX.Y.Z` on `main`.

## Provenance

Engine code started as an pp-cpp-ui 6.2 hard fork; it is now owned first-party source.
See [docs/PROVENANCE.md](docs/PROVENANCE.md).
