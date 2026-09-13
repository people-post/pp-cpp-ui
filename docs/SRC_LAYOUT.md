# Source layout

**Tier:** architecture

`pp-cpp-ui` is a shared C++ UI library. Dependencies flow downward only.

## Tree

```text
include/
  ui/                    # Public API (engine + owned host glue)
    Core/ Config/ SVG/ Debugger/
    platform/ render/    # SDL / GL3 backend headers
  RmlUi/                 # Compatibility shims → #include <ui/…>
src/
  core/ svg/ debugger/   # Engine implementation
  platform/ render/      # Owned SDL/GL backend
third_party/
cmake/
docs/
tests/
  engine/                # Unit tests
  support/               # Shell + SDL_GL3 reference backend only
  Tests -> engine        # Shim for legacy fixture virtual paths
```

## Include rules

| Consumer code | Preferred | Still works (shim) |
|---------------|-----------|--------------------|
| Engine types | `#include <ui/Core/…>` | `#include <RmlUi/Core/…>` |
| Owned backend | `#include <ui/platform/…>` | flat `RmlUi_Platform_SDL.h` shims |

Namespace remains `Rml::` for now (no consumer churn). C++ namespace brand rename is a later step.

## CMake targets

| Target | Alias | Role |
|--------|-------|------|
| `rmlui_core` | `RmlUi::Core` | Engine core (+ SVG / HarfBuzz) |
| `rmlui_debugger` | `RmlUi::Debugger` | Debugger |
| `pp_ui_rml` | `pp::ui_rml` | INTERFACE → core + `include/` |
| `pp_ui_backend` | `pp::ui_backend` | STATIC SDL/GL3 |
| `pp_ui` | `pp::ui` | Umbrella |

## Test data path shim

Unit tests still reference virtual paths under `../Tests/Data/...` relative to the
samples root (`tests/support/`). `tests/Tests` is a symlink to `tests/engine` so
those paths resolve without rewriting every fixture reference.
