# ADR 002 — Module dependency North Star

## Status

Accepted (on `cursor/first-party-layout-cc1d`).

## Context

After absorbing the engine into first-party modules (`base`, `style`, `layout`,
`dom`, …), folder split alone did not produce a DAG. Public and private includes
still form cycles and upward edges (`base→style`, `style↔dom`, `dom→core`,
`dom→widgets`, and widespread `Core.h` singleton use).

`docs/SRC_LAYOUT.md` previously said only “dependencies flow downward only”
without defining layers, allowed bridges, or enforcement.

## Decision

1. **Include DAG, one link target.** Layering is an include/dependency rule, not
   a requirement to split CMake libraries yet. `ui_core` remains one target.
2. **Strict layer stack** (higher may depend on lower; never reverse), documented
   in [SRC_LAYOUT.md](SRC_LAYOUT.md):

   `config → base → paint → style → layout → text → dom → xml → data → widgets → core → (svg|debugger) → (platform|render)`

3. **Hard rules**
   - **No cycles.** Break with a lower seam (interface, opaque handle, callback,
     or a tiny bridge owned by the lower layer).
   - **`core` is the composition root.** Plugins and backends may include `core`.
     Engine modules below `core` must not include `core` (inject interfaces via
     `Context` / constructors instead of `Core::Get*()`).
   - **`base` stays dumb.** No `base→style` (or any higher module).
   - **`style` does not own DOM.** No `style→dom`.
   - **`dom` does not know concrete widgets / xml / data.** Registration and
     parse/bind glue live in `core` or the higher module.
   - **Plugins and backends are leaves.** Engine modules never include
     `svg` / `debugger` / `platform` / `render` except `core` registering plugins.

4. **Named bridges (allowed upward)**
   - `text → dom` — `ElementText` and selection participation.
     (`layout → dom` cleared: layout uses `LayoutElement` façade; impl in `dom`.)

   These are explicit exceptions, not a license for other upward edges.

5. **Public and private includes obey the same DAG.**
   - Public: `#include <ui/module/Name.h>`
   - Private cross-module: `#include "module/Name.h"` with `-I src`

6. **Enforcement.** `scripts/check_module_deps.py` reports forbidden edges.
   Known legacy violations are allowlisted as tracked debt and must shrink over
   time; new forbidden edges fail the check.

## Consequences

- Refactors prioritize clearing allowlisted debt in this order:
  1. ~~`base → style`~~ (cleared: `Unit` / `Animation` / `DecorationTypes` live in `base`;
     style-only `TypeConverter` specializations live in `src/style/TypeConverterStyle.cpp`)
  2. ~~`style → dom` / `style → text` / `style → xml`~~ (cleared: Element-bound
     stylesheet/decorator/filter/transform/animation implementations and
     Stream-using loaders live under `src/dom/`; `style` keeps parsers/spec/value
     types. `ComputedValues` holds only an `Element*` and out-of-line accessors.)
  3. ~~most `* → core` via host getters~~ — `SystemInterface` / `FileInterface` (+ getters)
     live in `base`; `FontEngineInterface` / `TextInputHandler` getters live in `text`.
     Cleared `base|data|layout|paint|text|widgets|xml → core`.
  4. ~~`dom → core`~~ — `Plugin` / `PluginRegistry` live in `dom`; `ScriptInterface` in `base`;
     `Element.h` no longer includes `Core.h`.
  5. ~~`dom → widgets`~~ — default widget/data/XML control registration lives in
     `src/core/RegisterDefaultFactories.cpp`; `WidgetScroll` moved into `dom`.
  6. ~~`dom → xml`~~ — Stream* and `ParseDataBrackets` in `base`; document template types
     (`DocumentHeader` / `Template*` headers, `TemplateCache`) in `dom`; XMLParser-using
     `Template`/`DocumentHeader`/`Factory` stream impls live under `src/xml/`.
  7. ~~`dom → data`~~ — Factory/Context/Element data-binding implementations live under
     `src/data/` (`FactoryData`, `ContextData`, `ElementData`, `ElementUtilitiesData`).
  8. ~~`paint → layout` / `paint → text` / `paint → dom`~~ — unused paint includes
     removed; box-shadow cache/hash live in `dom`; `GeometryBoxShadow` keeps texture gen.
  9. ~~`text → widgets`~~ — `ElementSelectableText` / `ElementTextSelection` live in `text`.
  10. ~~`layout → text`~~ — `FontMetrics` in `base`; layout uses `LayoutTextElement` +
      `Element::GetFontMetrics()` / `GetAsLayoutTextElement()` instead of `ElementText`.
   11. Remaining named bridge involving DOM: `text → dom` only
     (`paint → style` also cleared with DecorationsTypes move).
  12. ~~`layout → dom`~~ — cleared: layout-owned `LayoutElement` API with
      implementation in `src/dom/LayoutElement.cpp` (see [LAYOUT_DOM_BRIDGE.md](LAYOUT_DOM_BRIDGE.md)).
- Optional later: split CMake targets to match layers once the include DAG is clean.
- Consumers see no API break from this ADR alone; breaks come only from follow-up
  refactors that move types between modules.
