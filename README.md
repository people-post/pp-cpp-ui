# pp-cpp-ui

Shared C++ UI stack for People Post apps: hard-forked [RmlUi](https://github.com/mikke89/RmlUi) plus FreeType, HarfBuzz, and LunaSVG.

## Contents

- `rmlui/` — owned RmlUi 6.2 hard fork (selectable text, UA sheet, list markers, …)
- `third_party/` — freetype, harfbuzz, lunasvg, zlib, libpng
- CMake targets: `pp_ui` / `pp::ui` (INTERFACE → `RmlUi::Core`), plus vendored `Freetype::Freetype`, `harfbuzz::harfbuzz`, `lunasvg::lunasvg`

**Not included** (stay in the app): SDL/GL backend, ShellHost/presenters, themes/views, fonts, emoji catalog/picker.

## Build

```bash
cmake -S . -B build -DPP_UI_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Standalone CI builds the RmlUi core library (and optional RmlUi tests). Full GUI apps still supply SDL/OpenGL themselves.

## Consume (FetchContent)

Pin a **release tag cut from `main`**:

```cmake
include(FetchContent)
FetchContent_Declare(
  pp_cpp_ui
  GIT_REPOSITORY https://github.com/people-post/pp-cpp-ui.git
  GIT_TAG v0.1.0
)
FetchContent_MakeAvailable(pp_cpp_ui)
target_link_libraries(your_target PUBLIC pp_ui)
# Paths: PP_LIB_RMLUI_ROOT / PP_LIB_RMLUI_INCLUDE
```

For a local sibling checkout (`../pp-cpp-ui`), pass `-DPP_CPP_UI_SOURCE_DIR=...` or rely on auto-detect from pp-browser.

Release flow: land on `develop` → merge to `main` → tag `vX.Y.Z` on `main`.
