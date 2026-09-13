#pragma once

#include <ui/Core/Element.h>

namespace ui {

class SelectionStyleClient {
public:
	virtual ~SelectionStyleClient() = default;
	virtual void OnSelectionStyleChanged() = 0;
};

/**
    A stub element used to query the RCSS-specified text colour and background colour for selected text.
 */

class ElementTextSelection : public Element {
public:
	UI_RTTI_DefineWithParent(ElementTextSelection, Element)

	ElementTextSelection(const String& tag);
	virtual ~ElementTextSelection();

	void SetClient(SelectionStyleClient* client);

protected:
	void OnPropertyChange(const PropertyIdSet& changed_properties) override;

private:
	SelectionStyleClient* client;
};

} // namespace ui
