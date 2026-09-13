# Source layout

**Tier:** architecture

`pp-cpp-ui` is a shared C++ UI library. Dependencies flow downward only.
The engine is one link target (`ui_core` / `ui::core`); sources and public headers
are split by module.

## Tree

```text
include/ui/
  config/            Build-time config
  base/              Types, math, containers, utilities
  style/             Properties, stylesheets, decorators, filters
  layout/            Box model
  dom/               Element, document, context, events, factory
  text/              Text, fonts, selection
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

## Include & namespace

```cpp
#include <ui/dom/Element.h>
// or umbrella:
#include <ui/Core.h>
ui::Element* el = ...;
```

Private engine headers use unqualified includes (`"LayoutEngine.h"`) with module
roots on `ui_core`'s private include path.

CMake targets: `ui::core`, `ui::debugger`, `ui::engine`, `pp::ui_core`, `pp::ui_backend`, `pp::ui`.

## Test data path

`tests/Tests` → `tests/engine` so fixture virtual paths under `../Tests/Data/...` still resolve.

## Private includes

Cross-module engine headers use a single `-I src` root and qualified paths:

```cpp
#include "text/SelectionController.h"
#include "debugger/Geometry.h"
#include "layout/LayoutEngine.h"
```

Same-folder includes (`#include "LayoutEngine.h"` from `layout/LayoutEngine.cpp`) stay unqualified.
Public headers always use `<ui/module/Name.h>`.
