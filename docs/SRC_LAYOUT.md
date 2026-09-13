# Source layout

**Tier:** architecture

`pp-cpp-ui` is a shared C++ UI library. The engine is one link target
(`ui_core` / `ui::core`); sources and public headers are split by module.
**Module dependencies form a DAG** — see [ADR 002](ADR_002_MODULE_DEPENDENCIES.md).

## Tree

```text
include/ui/
  config/            Build-time config
  base/              Types, math, containers, utilities, Unit/Animation/decoration values
  style/             Properties, stylesheets, decorators, filters
  layout/            Box model
  dom/               Element, document, context, events, factory
  text/              Fonts, shaping, selection, ElementText
  xml/               RML/XML streams & parsers
  data/              Data model
  paint/             Geometry, textures, render interfaces
  widgets/           Forms, inputs, tabset, progress
  core/              Bootstrap (Core, plugins, system/file interfaces)
  svg/ debugger/     Optional plugins
  platform/ render/  Owned SDL / GL3 backend headers
  Core.h Debugger.h  Convenience umbrellas
src/
  base style layout dom text xml data paint widgets core
  text/default text/harfbuzz
  svg debugger platform render
third_party/
cmake/
docs/
tests/
  engine/            Unit tests
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
L5   text
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
3. **`base` stays dumb.** No includes of `style` or any higher module.
   Value types used by `Variant` (`Unit`, `Animation`/`TransitionList`,
   `DecorationTypes`) live in `base`. Style-only `TypeConverter` specializations
   live in `src/style/TypeConverterStyle.cpp`.
4. **`style` does not own DOM.** No `style → dom` / `text` / `xml`. Element-bound
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
| `layout → dom` | Box queries / element layout façade |
| `text → dom` | `ElementText`, selection participation |

### Enforcement

```bash
python3 scripts/check_module_deps.py
```

Forbidden edges fail unless listed in the script’s allowlist (tracked debt).
Clear debt in the order in ADR 002; do not grow the allowlist without updating
the ADR.

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
#include "text/SelectionController.h"
#include "debugger/Geometry.h"
#include "layout/LayoutEngine.h"
```

Same-folder includes (`#include "LayoutEngine.h"` from `layout/LayoutEngine.cpp`)
stay unqualified. Public headers always use `<ui/module/Name.h>`.

## Test data path

`tests/Tests` → `tests/engine` so fixture virtual paths under `../Tests/Data/...`
still resolve.
