# ADR 001 — First-party source layout (absorb RmlUi)

## Status

Accepted (on `cursor/first-party-layout-cc1d`).

## Context

`pp-cpp-ui` previously kept an upstream-shaped hard fork under a root `rmlui/` tree (`Include/`, `Source/`, mega-option CMake) plus a flat `backend/` for SDL/GL3. That optimized for upstream re-imports, not for People Post ownership.

Sibling libs (`pp-cpp-common`, `pp-cpp-crypto`, `pp-cpp-amp`) use `include/<pkg>/` + `src/` + `third_party/` + `docs/` + `tests/`. Product apps (`pp-browser`, `pp-ledger`) use layered folders; those product layers do **not** belong in this library.

## Decision

1. Treat the UI engine as **first-party owned code**. No root `rmlui/` folder. Folder fidelity to upstream is not a goal.
2. Layout follows sibling libs: public headers under `include/`, implementation under `src/`, vendors under `third_party/`.
3. Public API is branded **`ui/`** includes and **`ui::`** namespace. No compatibility shims for `RmlUi/` or `Rml::`. Consumers (including pp-browser) update when they adopt this layout.
4. Drop unused upstream surface (Lua, Lottie, non-SDL reference backends, sample apps) as we absorb.
5. Upstream sync becomes **occasional cherry-picks**, not tree re-imports. Provenance is recorded in `docs/PROVENANCE.md`.

## Consequences

- CMake is rewritten around a fixed product profile (SVG + FreeType/HarfBuzz + static lib), not upstream’s option matrix.
- Breaking include/namespace change for all consumers — accepted; no shim layer.
- Re-merging full upstream releases becomes harder — accepted.

## Amendment — public include & namespace brand

Public engine headers live under `include/ui/<module>/` (`#include <ui/dom/Element.h>`, etc.).
C++ API uses `namespace ui` (macros `UI_*`). Former `include/RmlUi/` shims and `Rml::` are removed.

## Amendment — engine source & public header modules

Sources and public headers are split into focused modules while keeping a single
`ui_core` link target:

`base`, `style`, `layout`, `dom`, `text` (+ `default` / `harfbuzz`), `xml`, `data`,
`paint`, `widgets`, thin `core` bootstrap.

Public includes: `#include <ui/dom/Element.h>` (no `ui/Core/` prefix). Convenience
umbrella `#include <ui/Core.h>` remains.

## Amendment — private include root

Private engine includes use a single `-I src` root with module-qualified paths
(`#include "layout/LayoutEngine.h"`). Same-folder includes stay bare.
`SelectionController`’s full API is public under `include/ui/text/`; the private
header is a one-line redirect. This removes basename collisions from stacking
every `src/<module>` on the include path.

## Amendment — lowercase public folders

Public include directories use lowercase names (`config/`, `debugger/`, `svg/`,
`base/containers/`), matching `src/`. Umbrella headers `Core.h` / `Debugger.h` remain
at `include/ui/` for convenience.

## Amendment — module dependency North Star

Module include edges follow the DAG in [ADR 002](ADR_002_MODULE_DEPENDENCIES.md)
(`scripts/check_module_deps.py`).

## Amendment — style value types in base

`Unit`, `Animation` / `TransitionList`, and `DecorationTypes` (`ColorStop`,
`BoxShadow`) live under `include/ui/base/` so `base` does not include `style`.
Style-only `TypeConverter` specializations are implemented in
`src/style/TypeConverterStyle.cpp`.

## Amendment — style vs dom ownership

Element-bound style *application* (stylesheets against elements, decorator/filter
rendering, transform resolution, element animation, UA sheet load) lives under
`src/dom/`. The `style` module keeps property definitions, parsers, and
specifications without including `dom` / `text` / `xml`.


## Amendment — host interfaces in base/text

Host interfaces (`SystemInterface`, `FileInterface`) and their getters live in `base`; `FontEngineInterface` / `TextInputHandler` getters live in `text`. Cleared `base|data|layout|paint|text|widgets|xml → core`.
