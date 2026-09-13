# Source layout

**Tier:** architecture

`pp-cpp-ui` is a shared C++ UI library. Dependencies flow downward only.

**North Star:** engine core → optional engine plugins → SDL/GL backend → consumers (e.g. pp-browser host).

## Tree

```text
include/
  RmlUi/                 # Engine public API (kept prefix for consumer stability)
  ui/                    # Owned host glue public API (platform + render)
src/
  core/                  # Layout, elements, style, fonts, …
  svg/                   # SVG plugin (compiled into core when enabled)
  debugger/              # RmlUi debugger library
  platform/              # SystemInterface_SDL
  render/                # RenderInterface_GL3 (+ glad)
third_party/             # freetype, harfbuzz, lunasvg, zlib, libpng, sdl3, …
cmake/                   # PpUiVendors, PpUiProfile, engine helpers
docs/
tests/                   # Unit tests (+ thin shell harness when enabled)
scripts/
```

## CMake targets

| Target | Alias | Role |
|--------|-------|------|
| `rmlui_core` | `RmlUi::Core` | Engine core (+ SVG/HarfBuzz when profile on) |
| `rmlui_debugger` | `RmlUi::Debugger` | Debugger |
| `pp_ui_rml` / `pp_ui_core` | `pp::ui_rml` | INTERFACE → core (+ include root) |
| `pp_ui_backend` | `pp::ui_backend` | STATIC SDL/GL3 |
| `pp_ui` | `pp::ui` | Umbrella |

## Include rules

| Consumer code | Include |
|---------------|---------|
| Engine types | `#include <RmlUi/Core/…>` |
| Owned backend | `#include <ui/platform/…>` / `#include <ui/render/…>` |

Public include roots: `include/` only. Headers under `src/` are private.

## Dependency graph

```text
pp-browser foundation/platform/ui
  → pp::ui
      → pp::ui_backend          (src/platform, src/render)
          → RmlUi::Core         (src/core, src/svg, …)
              → third_party
      → RmlUi::Debugger         (optional)
```

## Not in this repo

Product window host, overlays, themes, views, fonts catalogs — stay in the app (`pp-browser`).

## Test data path shim

Unit tests still reference virtual paths under `../Tests/Data/...` relative to the
samples root (`tests/support/`). `tests/Tests` is a symlink to `tests/engine` so
those paths resolve without rewriting every fixture reference.

