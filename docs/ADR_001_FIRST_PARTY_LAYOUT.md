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

Public engine headers live under `include/ui/` (`#include <ui/Core/…>`).
C++ API uses `namespace ui` (macros `UI_*`). Former `include/RmlUi/` shims and `Rml::` are removed.

## Amendment — engine source modules

`src/core` was split into focused modules while keeping a single `ui_core` link target:

`base`, `style`, `layout`, `dom`, `text` (+ `default` / `harfbuzz`), `xml`, `data`, `paint`, `widgets`, thin `core` bootstrap.

Public headers remain under `include/ui/Core/` for now; mirroring into module folders is a follow-up.

