#include <ui/layout/LayoutElement.h>

#include <ui/base/Debug.h>
#include <ui/base/StringUtilities.h>
#include <ui/dom/Element.h>
#include <ui/dom/ElementScroll.h>
#include <ui/dom/ElementUtilities.h>

namespace ui {
namespace LayoutElement {
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

} // namespace LayoutElement
} // namespace ui
