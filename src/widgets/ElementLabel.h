#pragma once

#include <ui/dom/Element.h>
#include <ui/dom/EventListener.h>
#include <ui/base/Header.h>
namespace ui {

/**
    A specialisation of the generic Core::Element representing a label element.

 */

class ElementLabel : public Element, public EventListener {
public:
	UI_RTTI_DefineWithParent(ElementLabel, Element)

	ElementLabel(const String& tag);
	virtual ~ElementLabel();

protected:
	void OnPseudoClassChange(const String& pseudo_class, bool activate) override;

	void ProcessEvent(Event& event) override;

private:
	Element* GetTarget();

	bool disable_click = false;
};

} // namespace ui
