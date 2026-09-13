# Element as entity + parts

**Status:** in progress (Phase 1)  
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
3. **Split TUs** — animation / stacking / transform orchestration extracted from `Element.cpp`. ✓ (more TUs follow)
2. **Engine call sites** — migrate `src/` to parts-first.
3. **Split TUs** — move definitions to part-aligned `.cpp` files; `Element.cpp` keeps tree + Update/Render orchestration.
4. **Conservative `sizeof`** — cold fields allocate-on-use; keep BoxModel hot data in-line.
5. **Retire flat API** — remove duplicate Element methods once consumers moved.
6. **Controllers** — selection/focus/animation ownership on Document/Context.
7. **Layout port** — narrow `LayoutElement` toward BoxModel + style queries.

## Rules for new work

- No new product features as methods on `Element` when they belong on a part or controller.
- Do not pimpl the whole Element (hot path).
- Do not reintroduce `layout →` concrete Element includes outside `LayoutElement` / agreed ports.
