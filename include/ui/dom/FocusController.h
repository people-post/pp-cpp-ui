#pragma once

#include <ui/base/Header.h>
#include <ui/base/Types.h>

namespace ui {

class Context;
class Element;
class ElementDocument;

/**
    Owns context-level focus state and focus-change orchestration.

    Holds the focused element and document focus history, and runs blur/focus
    event dispatch for focus transitions (Element parts Phase 6b). Prefer
    `context->GetFocusController()` over reaching into Context members.
 */

class UI_CORE_API FocusController {
public:
	explicit FocusController(Context* context);

	Element* GetFocusElement() const { return focus; }
	void SetFocusElement(Element* element) { focus = element; }

	/// Apply a focus change: modal/unload checks, blur/focus events, history, z-order.
	/// @return False if the request was denied.
	bool OnFocusChange(Element* new_focus, bool focus_visible);

	void UnfocusDocument(ElementDocument* document);
	void OnDocumentUnload(ElementDocument* document);
	void OnElementDetach(Element* element);

	/// Push a document (or the context root) onto the focus history, moving it to the top if present.
	void PushDocument(Element* document);
	void RemoveDocument(Element* document);

	bool HasDocumentFocusHistory() const { return !document_focus_history.empty(); }
	Element* GetTopDocument() const;

private:
	Context* context = nullptr;
	Element* focus = nullptr;
	ElementList document_focus_history;
};

} // namespace ui
