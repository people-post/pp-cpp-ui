# Narrowing the `layout → dom` bridge

**Status:** in progress (Phase 1) on `cursor/first-party-layout-cc1d`  
**Related:** [ADR 002](ADR_002_MODULE_DEPENDENCIES.md), [SRC_LAYOUT.md](SRC_LAYOUT.md)

## Problem

ADR 002 allows `layout → dom` as a named bridge, but almost every layout
translation unit includes `Element.h` (and several also pull `ElementScroll`,
`ElementUtilities`, and private `ListMarker`). The bridge is a highway, not a
façade.

## Goal

Shrink the bridge until layout talks to a **small, layout-owned API surface**,
with at most one (or a few) `.cpp` files including concrete DOM headers. Ideal
end state: remove `layout → dom` from `ALLOWED_BRIDGES` if the remaining edge
is gone; otherwise keep a documented, minimal façade.

## Phases

### Phase 1 — Concentrate ancillary DOM helpers (done)

Introduce `ui/layout/LayoutElement.h` + `src/layout/LayoutElement.cpp` as the
layout-owned façade for:

- scrollbar enable/disable/size/format
- string width measurement
- list-marker prefix (fork workaround; move out of `src/dom/`)

Migrate layout call sites off `#include <ui/dom/ElementScroll.h>`,
`ElementUtilities.h`, and `"dom/ListMarker.h"`. `Element.h` may remain in
layout sources until Phase 2.

**Success:** those three include kinds appear only in `LayoutElement.cpp`
(or not at all).

### Phase 2 — Façade the hot Element ops (done)

Extend `LayoutElement` with the high-frequency Element operations layout uses
(`GetComputedValues`, box/offset submit, parent/child walk, replaced/intrinsic,
font metrics, `OnLayout`, debug name). Convert Inline* / LayoutDetails first,
then formatting contexts.

**Progress:** Every layout `.cpp` except the façade (`LayoutElement.cpp`) no longer
includes `Element.h`. Remaining Element traffic goes through `LayoutElement` +
`ElementAccess` (friend) for private layout hooks.

**Success:** most layout `.cpp` files no longer include `Element.h`; they use
an incomplete `Element*` plus façade calls (or a later `LayoutNode` handle).

### Phase 3 — Optional handle type

If Phase 2 still leaves awkward `Element*` noise, introduce a value/handle
`LayoutNode` in `layout/` that wraps `Element*` and returns `LayoutNode` for
parent/child. Escape hatch `Element* GetElement()` only at the composition
boundary (engine entry).

### Phase 4 — Revisit the named bridge

When layout sources no longer include `ui/dom/*`, drop `("layout", "dom")`
from `ALLOWED_BRIDGES` (or replace with a single allowlisted bridge file if a
tiny edge must remain).

## Non-goals

- Splitting CMake targets (still one `ui_core`)
- Removing `text → dom` (separate named bridge)
- Rewriting the layout algorithm itself
