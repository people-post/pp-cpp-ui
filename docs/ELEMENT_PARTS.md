# Element as entity + parts

**Status:** in progress (Phases 1–5c + 6a–6b landed; animation controller / layout port next)  
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

| Part | Type (today) | Presence | Accessor | Public header |
|------|----------------|----------|----------|---------------|
| Style | `ElementStyle` | Mandatory (Meta) | `Style()` | `ui/dom/ElementStyle.h` |
| Box | `ElementBox` (view → later storage) | Mandatory (hot) | `BoxModel()` | `ui/dom/ElementBox.h` |
| Scroll | `ElementScroll` | Mandatory (Meta) | `Scroll()` | `ui/dom/ElementScroll.h` |
| Events | `EventDispatcher` | Mandatory (Meta) | `Events()` | `ui/dom/EventDispatcher.h` |
| Effects | `ElementEffects` | Mandatory (Meta) | `Effects()` | `ui/dom/ElementEffects.h` |
| Background/border | `ElementBackgroundBorder` | Mandatory (Meta) | `BackgroundBorder()` | `ui/dom/ElementBackgroundBorder.h` |
| Transform | `TransformState` | Optional / cold | later `Transform()` | |
| Animation | animation list | Optional / cold | later `Animation()` | |

Session policy (focus path, selection gestures, animation clock) belongs on Document/Context controllers, not on Element.

## Controllers

| Controller | Owner | Role |
|------------|-------|------|
| `SelectionController` | `Context` | Static text selection gestures |
| `FocusController` | `Context` | Focused element + document focus history + blur/focus orchestration (Phase 6b) |

## Phases

1. **Expose parts** — accessors + `ElementBox` view; legacy getters alias; `sizeof` unchanged. ✓
2. **Engine call sites** — migrate `src/` to parts-first. ✓
3. **Split TUs** — animation, stacking, transform, geometry, tree, style façade, scroll API, events extracted. ✓
4. **Conservative `sizeof`** — cold fields allocate-on-use; hot BoxModel fields stay in-line. ✓ (`sizeof(Element)` 392 → 344)
5. **Retire flat API** — in progress:
   - **5a** ✓ Scroll offset/overflow on `ElementScroll`; Element façades thin; unused part-pointer aliases removed.
   - **5b** ✓ Publicize `ElementStyle` + `EventDispatcher` under `include/ui/dom/`; `Element.h` includes them so `Style()` / `Events()` are complete types. Engine call sites use `Style().SetProperty(PropertyId…)` / `Events().AttachEvent(EventId…)` where safe. Keep `Element::SetClass` / `SetPseudoClass` / string `SetProperty` / string `AddEventListener` façades (definition dirtying / name→id lookup).
   - **5c** ✓ Publicize `ElementEffects` + `ElementBackgroundBorder` under `include/ui/dom/`; `Element.h` / `Core.h` include them so `Effects()` / `BackgroundBorder()` are complete types.
6. **Controllers** — selection/focus/animation ownership on Document/Context.
   - **6a** ✓ `FocusController` owns focused element + document focus history; `Context::GetFocusController()`.
   - **6b** ✓ `FocusController::OnFocusChange` owns blur/focus event orchestration, modal/unload checks, document z-order, and history updates; `Context::OnFocusChange` is a thin delegate.
7. **Layout port** — narrow `LayoutElement` toward BoxModel + style queries.

## Rules for new work

- No new product features as methods on `Element` when they belong on a part or controller.
- Do not pimpl the whole Element (hot path).
- Do not reintroduce `layout →` concrete Element includes outside `LayoutElement` / agreed ports.
- Prefer `Style()` / `Events()` / `Scroll()` / `BoxModel()` / `Effects()` / `BackgroundBorder()` over flat Element methods when the part API covers the call.
- Prefer `Element::SetClass` / `SetPseudoClass` over `Style().SetClass` when sibling-combinator definition dirtying is required.
