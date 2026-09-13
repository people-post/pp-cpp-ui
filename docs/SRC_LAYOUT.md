# Source layout

**Tier:** architecture

`pp-cpp-ui` is a shared C++ UI library. Dependencies flow downward only.
The engine is one link target (`ui_core` / `ui::core`); sources are split by module.

## Tree

```text
include/ui/          Public API (still under Core/ Config/ SVG/ Debugger/ …)
  platform/ render/  Owned SDL / GL3 backend headers
src/
  base/              Math, memory, log, strings, pools, clocks
  style/             Properties, stylesheets, decorators, filters, transforms
  layout/            Formatting contexts / boxes
  dom/               Element, document, context, events, factory
  text/              Text, selection, font effects
    default/         FreeType font engine
    harfbuzz/        HarfBuzz shaping (uses default FreeType helpers)
  xml/               RML/XML parse, streams, templates
  data/              Data model / views / controllers
  paint/             Geometry, textures, render manager
  widgets/           Forms, inputs, tabset, images, …
  core/              Bootstrap (Core, plugins, system/file interfaces)
  svg/ debugger/
  platform/ render/  Product SDL / GL3 backend
third_party/
cmake/
docs/
tests/
  engine/            Unit tests
  support/           Shell + SDL_GL3 reference backend only
```

## Include & namespace

```cpp
#include <ui/Core/Element.h>
ui::Element* el = ...;
```

Private engine headers use unqualified includes (`"LayoutEngine.h"`) with all
module roots on `ui_core`'s private include path.

CMake targets: `ui::core`, `ui::debugger`, `ui::engine`, `pp::ui_core`, `pp::ui_backend`, `pp::ui`.

## Test data path

`tests/Tests` → `tests/engine` so fixture virtual paths under `../Tests/Data/...` still resolve.

## Follow-ups

Mirror public headers from `include/ui/Core/` into module folders (`include/ui/dom/`, …)
once consumers are ready for another include-path break.
