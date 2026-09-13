# Source layout

**Tier:** architecture

`pp-cpp-ui` is a shared C++ UI library. Dependencies flow downward only.

## Tree

```text
include/ui/          Public API
  Core/ Config/ SVG/ Debugger/
  platform/ render/  Owned SDL / GL3 backend headers
src/
  core/ svg/ debugger/
  platform/ render/
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

CMake targets: `ui::core`, `ui::debugger`, `ui::engine`, `pp::ui_core`, `pp::ui_backend`, `pp::ui`.

## Test data path

`tests/Tests` → `tests/engine` so fixture virtual paths under `../Tests/Data/...` still resolve.
