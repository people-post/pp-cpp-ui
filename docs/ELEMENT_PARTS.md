# Element as entity + parts

**Status:** in progress (Phases 1–5a landed; Style/Events public headers + remaining flat retirement next)  
**Related:** [ADR 002](ADR_002_MODULE_DEPENDENCIES.md), [LAYOUT_DOM_BRIDGE.md](LAYOUT_DOM_BRIDGE.md), [SRC_LAYOUT.md](SRC_LAYOUT.md)

## Charter

- `Element` is a **DOM entity**: identity, tree, lifecycle hooks, hot box cache, and accessors to parts.
- Algorithms live in **parts**, style/layout/paint engines, or Document/Context controllers — not on the base class as a growing god API.
- Preferred public model is parts-first:

  ```cpp
  element->Style().SetProperty(...);
  element->BoxModel().GetAbsoluteOffset(...);
  element->Scroll().FormatScrollbars();
  element->Events().AttachEvent(...);
  element->Effects().DirtyEffects();
  element->BackgroundBorder().Render(element);
  ```

- Flat `Element::SetProperty` / `GetScrollLeft` / … remain during migration; new features must not extend that surface.
- Module DAG unchanged. Layout talks through `LayoutElement` (later: BoxModel port), not the whole Element API.

## Parts

| Part | Type (today) | Presence | Accessor |
|------|----------------|----------|----------|
| Style | `ElementStyle` | Mandatory (Meta) | `Style()` |
| Box | `ElementBox` (view → later storage) | Mandatory (hot) | `BoxModel()` |
| Scroll | `ElementScroll` | Mandatory (Meta) | `Scroll()` |
| Events | `EventDispatcher` | Mandatory (Meta) | `Events()` |
| Effects | `ElementEffects` | Mandatory (Meta) | `Effects()` |
| Background/border | `ElementBackgroundBorder` | Mandatory (Meta) | `BackgroundBorder()` |
| Transform | `TransformState` | Optional / cold | later `Transform()` |
| Animation | animation list | Optional / cold | later `Animation()` |

Session policy (focus path, selection gestures, animation clock) belongs on Document/Context controllers, not on Element.

## Phases

1. **Expose parts** — accessors + `ElementBox` view; legacy getters alias; `sizeof` unchanged. ✓
2. **Engine call sites** — migrate `src/` to parts-first. ✓
3. **Split TUs** — animation, stacking, transform, geometry, tree, style façade, scroll API, events extracted. ✓ (`Element.cpp` ~1k lines; further attribute/lifecycle optional)
4. **Conservative `sizeof`** — `animations`, `stacking_context`, `additional_boxes` are `UniquePtr` (allocate-on-use); hot BoxModel fields stay in-line. ✓ (`sizeof(Element)` 392 → 344)
5. **Retire flat API** — in progress:
   - **5a** ✓ Scroll offset/overflow APIs live on `ElementScroll`; Element methods are thin façades (`Prefer Scroll()`). Engine call sites use `Scroll()` / `BoxModel()` where types are public. Removed unused `GetElementScroll` / `GetElementEffects` / `GetElementBackgroundBorder` / `GetEventDispatcher` aliases.
   - **5b** (next) Publicize `ElementStyle` (and `EventDispatcher` if needed) under `include/ui/…` so `Style()` / `Events()` are usable outside `src/dom/`, then migrate/remove flat style & event façades.
6. **Controllers** — selection/focus/animation ownership on Document/Context.
7. **Layout port** — narrow `LayoutElement` toward BoxModel + style queries.

## Rules for new work

- No new product features as methods on `Element` when they belong on a part or controller.
- Do not pimpl the whole Element (hot path).
- Do not reintroduce `layout →` concrete Element includes outside `LayoutElement` / agreed ports.
- Do not call `Style().…` from TUs that only see a forward-declared `ElementStyle` (incomplete type) until 5b lands — use Element façades or include the style header.
