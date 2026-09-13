# ADR 001 — First-party source layout (absorb pp-cpp-ui)

## Status

Accepted (in progress on `cursor/first-party-layout-cc1d`).

## Context

`pp-cpp-ui` previously kept an upstream-shaped hard fork under a root `rmlui/` tree (`Include/`, `Source/`, mega-option CMake) plus a flat `backend/` for SDL/GL3. That optimized for upstream re-imports, not for People Post ownership.

Sibling libs (`pp-cpp-common`, `pp-cpp-crypto`, `pp-cpp-amp`) use `include/<pkg>/` + `src/` + `third_party/` + `docs/` + `tests/`. Product apps (`pp-browser`, `pp-ledger`) use layered folders; those product layers do **not** belong in this library.

## Decision

1. Treat the UI engine as **first-party owned code**. No root `rmlui/` folder. Folder fidelity to upstream is not a goal.
2. Layout follows sibling libs: public headers under `include/`, implementation under `src/`, vendors under `third_party/`.
3. Keep the public **`pp-cpp-ui/` include prefix** and `ui::` namespace for now (consumer stability). Optional brand rename to `ui/` / `pp::ui` is a later phase.
4. Drop unused upstream surface (Lua, Lottie, non-SDL reference backends, sample apps) as we absorb.
5. Upstream sync becomes **occasional cherry-picks**, not tree re-imports. Provenance is recorded in `docs/PROVENANCE.md`.

## Consequences

- CMake is rewritten around a fixed product profile (SVG + FreeType/HarfBuzz + static lib), not upstream’s option matrix.
- pp-browser keeps `#include <pp-cpp-ui/…>`; only owned backend includes may move to `ui/…`.
- Re-merging full upstream releases becomes harder — accepted.

## Amendment — public include brand

Public engine headers live under `include/ui/` (`#include <ui/Core/…>`).
`include/ui/` remains as compatibility shims so existing consumers keep compiling
without source changes. `namespace ui` is unchanged in this step.

