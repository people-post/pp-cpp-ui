# Narrowing the `layout → dom` bridge

**Status:** complete on `cursor/first-party-layout-cc1d`  
**Related:** [ADR 002](ADR_002_MODULE_DEPENDENCIES.md), [SRC_LAYOUT.md](SRC_LAYOUT.md)

## Problem

ADR 002 originally allowed `layout → dom` as a named bridge, but almost every
layout translation unit included `Element.h` (and several also pulled
`ElementScroll`, `ElementUtilities`, and private list-marker helpers). The
bridge was a highway, not a façade.

## Goal

Shrink the bridge until layout talks to a **small, layout-owned API surface**,
with layout sources never including concrete DOM headers. Ideal end state:
remove `layout → dom` from `ALLOWED_BRIDGES`.

## Phases

### Phase 1 — Concentrate ancillary DOM helpers (done)

Introduced `ui/layout/LayoutElement.h` + implementation for:

- scrollbar enable/disable/size/format
- string width measurement
- list-marker prefix (fork workaround; moved out of `src/dom/ListMarker.*`)

### Phase 2 — Façade the hot Element ops (done)

Extended `LayoutElement` with high-frequency Element operations. Every layout
`.cpp` stopped including `Element.h` except the façade implementation.

### Phase 3 — Optional `LayoutNode` handle (deferred)

A value/handle wrapping `Element*` remains optional. The free-function façade
already removed include coupling; a handle type can wait until call-site noise
justifies the churn.

### Phase 4 — Drop the named bridge (done)

Moved the façade implementation to `src/dom/LayoutElement.cpp` (dom may depend
on layout). Layout keeps only the header + `Element` forward declaration.

**Result:** no `src/layout/**` file includes `ui/dom/*`. Removed
`("layout", "dom")` from `ALLOWED_BRIDGES`.

**Follow-up (done):** `text → dom` cleared by moving `ElementText` and selection
UI into `dom/`; `font/` is font/shaping only. No upward bridges into `dom` remain.

## Non-goals

- Splitting CMake targets (still one `ui_core`)
- Further `LayoutNode` handle churn (still deferred)
- Rewriting the layout algorithm itself
