#include <ui/dom/FocusController.h>
#include <ui/dom/Element.h>
#include <ui/dom/ElementDocument.h>
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

} // namespace ui
