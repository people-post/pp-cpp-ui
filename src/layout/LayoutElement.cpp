#include <ui/layout/LayoutElement.h>

#include <ui/base/Debug.h>
#include <ui/base/StringUtilities.h>
#include <ui/dom/Element.h>
#include <ui/dom/ElementScroll.h>
#include <ui/dom/ElementUtilities.h>
#include <ui/style/ComputedValues.h>

namespace ui {
namespace LayoutElement {

class ElementAccess {
public:
	static void UpdateOffset(Element* element) { element->UpdateOffset(); }
	static void SetBaseline(Element* element, float baseline) { element->SetBaseline(baseline); }
	static void OnLayout(Element* element) { element->OnLayout(); }
	static void ClampScrollOffsetRecursive(Element* element) { element->ClampScrollOffsetRecursive(); }
};

namespace {

ElementScroll::Orientation ToScrollOrientation(LayoutScrollbarAxis axis)
{
	return axis == LayoutScrollbarAxis::Horizontal ? ElementScroll::HORIZONTAL : ElementScroll::VERTICAL;
}

} // namespace

float GetScrollbarSize(Element* element, LayoutScrollbarAxis axis)
{
	UI_ASSERT(element);
	return element->GetElementScroll()->GetScrollbarSize(ToScrollOrientation(axis));
}

void EnableScrollbar(Element* element, LayoutScrollbarAxis axis, float element_width)
{
	UI_ASSERT(element);
	element->GetElementScroll()->EnableScrollbar(ToScrollOrientation(axis), element_width);
}

void DisableScrollbar(Element* element, LayoutScrollbarAxis axis)
{
	UI_ASSERT(element);
	element->GetElementScroll()->DisableScrollbar(ToScrollOrientation(axis));
}

void FormatScrollbars(Element* element)
{
	UI_ASSERT(element);
	element->GetElementScroll()->FormatScrollbars();
}

float GetStringWidth(Element* element, StringView string)
{
	UI_ASSERT(element);
	return float(ElementUtilities::GetStringWidth(element, string));
}

String GetListItemMarker(Element* list_item_element)
{
	// FORK_WORKAROUND: browsers use list-style / ::marker; this injects marker text during
	// inline layout. Limitations: direct <li>text</li> only; fixed • / "N. " styles.
	if (!list_item_element || list_item_element->GetTagName() != "li")
		return {};

	Element* list = list_item_element->GetParentNode();
	if (!list)
		return {};

	const String& list_tag = list->GetTagName();
	if (list_tag != "ul" && list_tag != "ol")
		return {};

	int index = 0;
	for (int i = 0; i < list->GetNumChildren(); i++)
	{
		Element* child = list->GetChild(i);
		if (child->GetTagName() != "li")
			continue;

		index++;
		if (child == list_item_element)
		{
			if (list_tag == "ol")
				return CreateString("%d. ", index);
			return String("\xE2\x80\xA2 ");
		}
	}

	return {};
}

const ComputedValues& GetComputedValues(Element* element)
{
	UI_ASSERT(element);
	return element->GetComputedValues();
}

Style::Display GetDisplay(Element* element)
{
	UI_ASSERT(element);
	return element->GetDisplay();
}

Style::Position GetPosition(Element* element)
{
	UI_ASSERT(element);
	return element->GetPosition();
}

float GetLineHeight(Element* element)
{
	UI_ASSERT(element);
	return element->GetLineHeight();
}

const FontMetrics& GetFontMetrics(Element* element)
{
	UI_ASSERT(element);
	return element->GetFontMetrics();
}

const Box& GetBox(Element* element)
{
	UI_ASSERT(element);
	return element->GetBox();
}

void SetBox(Element* element, const Box& box)
{
	UI_ASSERT(element);
	element->SetBox(box);
}

void AddBox(Element* element, const Box& box, Vector2f offset)
{
	UI_ASSERT(element);
	element->AddBox(box, offset);
}

void SetOffset(Element* element, Vector2f offset, Element* offset_parent, bool offset_fixed)
{
	UI_ASSERT(element);
	element->SetOffset(offset, offset_parent, offset_fixed);
}

void UpdateOffset(Element* element)
{
	UI_ASSERT(element);
	ElementAccess::UpdateOffset(element);
}

void SetBaseline(Element* element, float baseline)
{
	UI_ASSERT(element);
	ElementAccess::SetBaseline(element, baseline);
}

float GetBaseline(Element* element)
{
	UI_ASSERT(element);
	return element->GetBaseline();
}

void OnLayout(Element* element)
{
	UI_ASSERT(element);
	ElementAccess::OnLayout(element);
}

void ClampScrollOffsetRecursive(Element* element)
{
	UI_ASSERT(element);
	ElementAccess::ClampScrollOffsetRecursive(element);
}

void SetScrollableOverflowRectangle(Element* element, Vector2f scrollable_overflow_rectangle, bool clamp_scroll_offset)
{
	UI_ASSERT(element);
	element->SetScrollableOverflowRectangle(scrollable_overflow_rectangle, clamp_scroll_offset);
}

Element* GetParentNode(Element* element)
{
	UI_ASSERT(element);
	return element->GetParentNode();
}

Element* GetChild(Element* element, int index)
{
	UI_ASSERT(element);
	return element->GetChild(index);
}

int GetNumChildren(Element* element, bool include_non_dom_elements)
{
	UI_ASSERT(element);
	return element->GetNumChildren(include_non_dom_elements);
}

Element* GetOffsetParent(Element* element)
{
	UI_ASSERT(element);
	return element->GetOffsetParent();
}

Vector2f GetRelativeOffset(Element* element, BoxArea area)
{
	UI_ASSERT(element);
	return element->GetRelativeOffset(area);
}

bool IsReplaced(Element* element)
{
	UI_ASSERT(element);
	return element->IsReplaced();
}

bool GetIntrinsicDimensions(Element* element, Vector2f& dimensions, float& ratio)
{
	UI_ASSERT(element);
	return element->GetIntrinsicDimensions(dimensions, ratio);
}

const String& GetId(Element* element)
{
	UI_ASSERT(element);
	return element->GetId();
}

const String& GetTagName(Element* element)
{
	UI_ASSERT(element);
	return element->GetTagName();
}

String GetAddress(Element* element, bool include_pseudo_classes, bool include_parents)
{
	UI_ASSERT(element);
	return element->GetAddress(include_pseudo_classes, include_parents);
}

bool HasAttribute(Element* element, const String& name)
{
	UI_ASSERT(element);
	return element->HasAttribute(name);
}

String GetAttributeString(Element* element, const String& name, const String& default_value)
{
	UI_ASSERT(element);
	return element->GetAttribute(name, default_value);
}

int GetAttributeInt(Element* element, const String& name, int default_value)
{
	UI_ASSERT(element);
	return element->GetAttribute(name, default_value);
}

LayoutTextElement* GetAsLayoutTextElement(Element* element)
{
	UI_ASSERT(element);
	return element->GetAsLayoutTextElement();
}

} // namespace LayoutElement
} // namespace ui
