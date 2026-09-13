#include <ui/dom/FocusController.h>
#include <ui/dom/Context.h>
#include <ui/dom/Element.h>
#include <ui/dom/ElementDocument.h>
#include <ui/style/ComputedValues.h>
#include <algorithm>

namespace ui {

FocusController::FocusController(Context* _context) : context(_context) {}

Element* FocusController::GetTopDocument() const
{
	if (document_focus_history.empty())
		return nullptr;
	return document_focus_history.back();
}

void FocusController::PushDocument(Element* document)
{
	if (!document)
		return;
	RemoveDocument(document);
	document_focus_history.push_back(document);
}

void FocusController::RemoveDocument(Element* document)
{
	auto it = std::find(document_focus_history.begin(), document_focus_history.end(), document);
	if (it != document_focus_history.end())
		document_focus_history.erase(it);
}

void FocusController::UnfocusDocument(ElementDocument* document)
{
	RemoveDocument(document);
	if (!document_focus_history.empty())
		document_focus_history.back()->GetFocusLeafNode()->Focus();
}

void FocusController::OnDocumentUnload(ElementDocument* document)
{
	RemoveDocument(document);

	if (focus && focus->GetOwnerDocument() == document)
	{
		focus = nullptr;
		if (!document_focus_history.empty())
			document_focus_history.back()->GetFocusLeafNode()->Focus();
	}
}

void FocusController::OnElementDetach(Element* element)
{
	if (element == focus)
		focus = nullptr;

	// Documents appear as self-owned documents in the tree.
	if (element->GetOwnerDocument() == element)
		RemoveDocument(element);
}

bool FocusController::OnFocusChange(Element* new_focus, bool focus_visible)
{
	UI_ASSERT(new_focus);
	UI_ASSERT(context);

	Context::ElementSet old_chain;
	Context::ElementSet new_chain;

	Element* old_focus = focus;
	ElementDocument* old_document = old_focus ? old_focus->GetOwnerDocument() : nullptr;
	ElementDocument* new_document = new_focus->GetOwnerDocument();

	// If the current focus is modal and the new focus cannot receive focus from modal, deny the request.
	if (old_document && old_document->IsModal() && (!new_document || !(new_document->IsModal() || new_document->IsFocusableFromModal())))
		return false;

	// If the document of the new focus has been closed, deny the request.
	if (std::find_if(context->unloaded_documents.begin(), context->unloaded_documents.end(),
			[&](const auto& unloaded_document) { return unloaded_document.get() == new_document; }) != context->unloaded_documents.end())
	{
		return false;
	}

	// Build the old chain
	Element* element = old_focus;
	while (element)
	{
		old_chain.insert(element);
		element = element->GetParentNode();
	}

	// Build the new chain
	element = new_focus;
	while (element)
	{
		new_chain.insert(element);
		element = element->GetParentNode();
	}

	// Send out blur/focus events.
	Dictionary parameters;
	Context::SendEvents(old_chain, new_chain, EventId::Blur, parameters);

	if (focus_visible)
		parameters["focus_visible"] = true;

	Context::SendEvents(new_chain, old_chain, EventId::Focus, parameters);

	focus = new_focus;

	// Raise the element's document to the front, if desired.
	ElementDocument* document = new_focus->GetOwnerDocument();
	if (document != nullptr)
	{
		Style::ZIndex z_index_property = document->GetComputedValues().z_index();
		if (z_index_property.type == Style::ZIndex::Auto)
			document->PullToFront();
	}

	// Update the focus history
	if (old_document != new_document && new_document != nullptr)
		PushDocument(new_document);

	return true;
}

} // namespace ui
