#include <ui/dom/ElementBox.h>
#include <ui/dom/Element.h>

namespace ui {

void ElementBox::SetOffset(Vector2f offset, Element* offset_parent, bool offset_fixed)
{
	element->SetOffset(offset, offset_parent, offset_fixed);
}

Vector2f ElementBox::GetRelativeOffset(BoxArea area)
{
	return element->GetRelativeOffset(area);
}

Vector2f ElementBox::GetAbsoluteOffset(BoxArea area)
{
	return element->GetAbsoluteOffset(area);
}

void ElementBox::SetClipArea(BoxArea clip_area)
{
	element->SetClipArea(clip_area);
}

BoxArea ElementBox::GetClipArea() const
{
	return element->GetClipArea();
}

void ElementBox::SetScrollableOverflowRectangle(Vector2f scrollable_overflow_rectangle, bool clamp_scroll_offset)
{
	element->SetScrollableOverflowRectangle(scrollable_overflow_rectangle, clamp_scroll_offset);
}

void ElementBox::SetBox(const Box& box)
{
	element->SetBox(box);
}

void ElementBox::AddBox(const Box& box, Vector2f offset)
{
	element->AddBox(box, offset);
}

const Box& ElementBox::GetBox()
{
	return element->GetBox();
}

const Box& ElementBox::GetBox(int index, Vector2f& offset)
{
	return element->GetBox(index, offset);
}

RenderBox ElementBox::GetRenderBox(BoxArea fill_area, int index)
{
	return element->GetRenderBox(fill_area, index);
}

int ElementBox::GetNumBoxes()
{
	return element->GetNumBoxes();
}

} // namespace ui
