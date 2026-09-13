# Source layout

**Tier:** architecture

`pp-cpp-ui` is a shared C++ UI library. The engine is one link target
(`ui_core` / `ui::core`); sources and public headers are split by module.
**Module dependencies form a DAG** — see [ADR 002](ADR_002_MODULE_DEPENDENCIES.md).

## Tree

```text
include/ui/
  config/            Build-time config
  base/              Types, math, containers, utilities, Unit/Animation/decoration values, Input enums, host TextInput*/TextLoupe
  style/             Properties, stylesheets, decorators, filters
  layout/            Box model, LayoutTextElement seam
  dom/               Element, document, context, events, factory, ElementText, selection
  font/              Fonts, shaping, font effects (ElementText / selection in dom; TextInput* in base)
  xml/               RML/XML streams & parsers
  data/              Data model
  paint/             Geometry, textures, render interfaces
  widgets/           Forms, inputs, tabset, progress
  core/              Bootstrap (Core, plugins, system/file interfaces)
  svg/ debugger/     Optional plugins
  platform/ render/  Owned SDL / GL3 backend headers
  Core.h Debugger.h  Convenience umbrellas
src/
  base style layout dom font xml data paint widgets core
  font/default font/harfbuzz
  svg debugger platform render
  <module>/tests/    Module unit tests (`*_test.cpp`)
third_party/
cmake/
docs/
tests/
  engine/            Runner, harness (Common), fixtures, visual/bench
  support/           Shell + SDL_GL3 reference backend only
```

## Module dependency North Star

Layers (top may depend on lower; **never** reverse):

```text
L0   config
L1   base
L2   paint
L3   style
L4   layout
L5   font
L6   dom
L7   xml
L8   data
L9   widgets
L10  core                 composition root
L11  svg | debugger      optional plugins
L12  platform | render    owned backends
```

### Hard rules

1. **No cycles.** If A needs B and B needs A, extract a lower seam.
2. **`core` is the composition root.** Only `core`, plugins (`svg`, `debugger`),
   and backends (`platform`, `render`) may include `core`. Lower modules must
   not call `Core::Get*()`; prefer injection via `Context` / interfaces.
3. **`base` stays dumb.** Host `SystemInterface` / `FileInterface` (+ getters) live in `base`. No includes of `style` or any higher module.
   Value types used by `Variant` (`Unit`, `Animation`/`TransitionList`,
   `DecorationTypes`) live in `base`. Style-only `TypeConverter` specializations
   live in `src/style/TypeConverterStyle.cpp`.
4. **`style` does not own DOM.** No `style → dom` / `font` / `xml`. Element-bound
   stylesheet application, decorators/filters, and transform resolution live in
   `src/dom/`; `style` owns property/spec/parser value machinery.
5. **`dom` does not know concrete widgets, xml, or data.** Factory registration
   and parse/bind glue belong in `core` or the higher module.
6. **Plugins and backends are leaves.** Engine modules never include them
   except `core` registering plugins.
7. **Same-layer peers** (`svg`↔`debugger`, `platform`↔`render`) do not include
   each other unless we document a directed edge.
8. **Public and private includes obey the same DAG.**

### Named bridges (allowed upward)

| Edge | Why |
|------|-----|
(none into `dom`)

`layout → dom` and former `text → dom` cleared: layout uses `LayoutElement`; `ElementText` /
selection live in `dom`. Module `font/` (renamed from `text/`) is font/shaping only; `SelectionTypes` lives in `dom/`, host `TextInput*` / `TextLoupe` in `base/`
(see [LAYOUT_DOM_BRIDGE.md](LAYOUT_DOM_BRIDGE.md)).

### Enforcement

```bash
python3 scripts/check_module_deps.py
```

Forbidden edges fail the check. The allowlist is empty after clearing ADR 002
tracked debt; no upward bridges into `dom` remain (`core → svg|debugger` only at the composition root).
Do not grow a new allowlist without updating the ADR.

## Include & namespace

```cpp
#include <ui/dom/Element.h>
// or umbrella:
#include <ui/Core.h>
ui::Element* el = ...;
```

CMake targets: `ui::core`, `ui::debugger`, `ui::engine`, `pp::ui_core`,
`pp::ui_backend`, `pp::ui`.

## Private includes

Cross-module engine headers use a single `-I src` root and qualified paths:

```cpp
#include "debugger/Geometry.h"
#include "layout/LayoutEngine.h"
#include <ui/dom/SelectionController.h>  // public headers use <ui/…>
```

Same-folder includes (`#include "LayoutEngine.h"` from `layout/LayoutEngine.cpp`)
stay unqualified. Public headers always use `<ui/module/Name.h>`.

## Unit tests

Colocate module-owned unit tests under `src/<module>/tests/*_test.cpp` (same
module DAG as production code). Keep shared harness, fixtures, visual tests,
and benchmarks under `tests/`. One executable `ui_unit_tests` (doctest) lists
those sources from `tests/engine/Source/UnitTests/CMakeLists.txt`.

| Put next to source | Keep under `tests/` |
|--------------------|---------------------|
| Pure / module-primary unit tests | Harness (`Source/Common`), `Data/`, visual, benchmarks, deps |
| One primary module under test | Cross-cutting runner (`main.cpp`) |

Harness headers (`TestsShell.h`, …) come from `ui_tests_common`’s INTERFACE
include path — tests `#include "TestsShell.h"` (not a relative `../Common/`).

## Test data path

`tests/Tests` → `tests/engine` so fixture virtual paths under `../Tests/Data/...`
still resolve.

## Element entity + parts

`Element` is evolving toward an entity + parts model (preferred accessors
`Style()`, `BoxModel()`, `Scroll()`, `Events()`, `Effects()`, `BackgroundBorder()`).
See [ELEMENT_PARTS.md](ELEMENT_PARTS.md).

## Owned static bulk (light headers)

Large static payloads stay in-repo without a generator, but out of hot headers:

| Owned artifact | Location | Header strategy |
|----------------|----------|-----------------|
| Debugger fonts | `src/debugger/FontSource.cpp` | Light `FontSource.h` (`extern` only) |
| GLAD GL 3.3 loader | `src/render/gl/Include_GL3.h` + `glad.cpp` | Render-private; impl in its own TU |
| Hash containers | `src/base/containers/` | Included via `Config.h` aliases; see `OWNERSHIP.md` |

## CMake libraries (one per module folder)

Each `src/<module>/` builds an **OBJECT** library `ui_<module>` (`ui::base`, `ui::dom`, …)
with PUBLIC usage requirements following the ADR DAG. Objects are folded into the
umbrella **STATIC** `ui_core` (`ui::core` / `pp_ui_core` / `pp_ui`, optional
`ui::debugger`) so consumers (pp-browser) keep a single link line and we avoid
static-archive cycles (e.g. layout↔dom). Compat alias **`RmlUi::Core`** → `ui_core`.
