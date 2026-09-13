// Element — events / focus façade (definitions only).
#include <ui/dom/Element.h>
#include <ui/dom/ElementDocument.h>
#include <ui/dom/Event.h>
#include "ElementMeta.h"
#include "EventDispatcher.h"
#include "EventSpecification.h"

namespace ui {

Element* Element::GetFocusLeafNode()
{
	// If there isn't a focus, then we are the leaf.
	if (!focus)
	{
		return this;
	}

	// Recurse down the tree until we found the leaf focus element
	Element* focus_element = focus;
	while (focus_element->focus)
		focus_element = focus_element->focus;

	return focus_element;
}
bool Element::Focus(bool focus_visible)
{
	// Are we allowed focus?
	Style::Focus focus_property = meta->computed_values.focus();
	if (focus_property == Style::Focus::None)
		return false;

	// Ask our context if we can switch focus.
	Context* context = GetContext();
	if (context == nullptr)
		return false;

	if (!context->OnFocusChange(this, focus_visible))
		return false;

	// Set this as the end of the focus chain.
	focus = nullptr;

	// Update the focus chain up the hierarchy.
	Element* element = this;
	while (Element* parent = element->GetParentNode())
	{
		parent->focus = element;
		element = parent;
	}

	return true;
}
void Element::Blur()
{
	if (parent)
	{
		Context* context = GetContext();
		if (context == nullptr)
			return;

		if (context->GetFocusElement() == this)
		{
			parent->Focus();
		}
		else if (parent->focus == this)
		{
			parent->focus = nullptr;
		}
	}
}
void Element::Click()
{
	Context* context = GetContext();
	if (context == nullptr)
		return;

	context->GenerateClickEvent(this);
}
void Element::AddEventListener(const String& event, EventListener* listener, const bool in_capture_phase)
{
	const EventId id = EventSpecificationInterface::GetIdOrInsert(event);
	Events().AttachEvent(id, listener, in_capture_phase);
}
void Element::AddEventListener(const EventId id, EventListener* listener, const bool in_capture_phase)
{
	Events().AttachEvent(id, listener, in_capture_phase);
}
void Element::RemoveEventListener(const String& event, EventListener* listener, bool in_capture_phase)
{
	EventId id = EventSpecificationInterface::GetIdOrInsert(event);
	Events().DetachEvent(id, listener, in_capture_phase);
}
void Element::RemoveEventListener(EventId id, EventListener* listener, bool in_capture_phase)
{
	Events().DetachEvent(id, listener, in_capture_phase);
}
bool Element::DispatchEvent(const String& type, const Dictionary& parameters)
{
	const EventSpecification& specification = EventSpecificationInterface::GetOrInsert(type);
	return EventDispatcher::DispatchEvent(this, specification.id, type, parameters, specification.interruptible, specification.bubbles,
		specification.default_action_phase);
}
bool Element::DispatchEvent(const String& type, const Dictionary& parameters, bool interruptible, bool bubbles)
{
	const EventSpecification& specification = EventSpecificationInterface::GetOrInsert(type);
	return EventDispatcher::DispatchEvent(this, specification.id, type, parameters, interruptible, bubbles, specification.default_action_phase);
}
bool Element::DispatchEvent(EventId id, const Dictionary& parameters)
{
	const EventSpecification& specification = EventSpecificationInterface::Get(id);
	return EventDispatcher::DispatchEvent(this, specification.id, specification.type, parameters, specification.interruptible, specification.bubbles,
		specification.default_action_phase);
}
void Element::ProcessDefaultAction(Event& event)
{
	if (event == EventId::Mousedown)
	{
		const Vector2f mouse_pos(event.GetParameter("mouse_x", 0.f), event.GetParameter("mouse_y", 0.f));

		if (IsPointWithinElement(mouse_pos) && event.GetParameter("button", 0) == 0)
			SetPseudoClass("active", true);
	}

	if (event.GetPhase() == EventPhase::Target)
	{
		switch (event.GetId())
		{
		case EventId::Mouseover: SetPseudoClass("hover", true); break;
		case EventId::Mouseout: SetPseudoClass("hover", false); break;
		case EventId::Focus:
			SetPseudoClass("focus", true);
			if (event.GetParameter("focus_visible", false))
				SetPseudoClass("focus-visible", true);
			break;
		case EventId::Blur:
			SetPseudoClass("focus", false);
			SetPseudoClass("focus-visible", false);
			break;
		default: break;
		}
	}
}

} // namespace ui
