// Element — scroll API (definitions only).
#include <ui/dom/Element.h>
#include <ui/dom/ElementDocument.h>
#include <ui/dom/ElementScroll.h>
#include <ui/style/ComputedValues.h>
#include <ui/base/Math.h>
#include "ElementMeta.h"

namespace ui {

static float GetScrollOffsetDelta(ScrollAlignment alignment, float begin_offset, float end_offset)
{
	switch (alignment)
	{
	case ScrollAlignment::Start: return begin_offset;
	case ScrollAlignment::Center: return (begin_offset + end_offset) / 2.0f;
	case ScrollAlignment::End: return end_offset;
	case ScrollAlignment::Nearest:
		if (begin_offset >= 0.f && end_offset <= 0.f)
			return 0.f; // Element is already visible, don't scroll
		else if (begin_offset < 0.f && end_offset < 0.f)
			return Math::Max(begin_offset, end_offset);
		else if (begin_offset > 0.f && end_offset > 0.f)
			return Math::Min(begin_offset, end_offset);
		else
			return 0.f; // Shouldn't happen
	}
	return 0.f;
}
float Element::GetScrollLeft()
{
	return scroll_offset.x;
}
void Element::SetScrollLeft(float scroll_left, bool clamp)
{
	const float max_scroll = Math::Max(0.0f, GetScrollWidth() - GetClientWidth());
	const float new_offset = Math::Round(clamp ? Math::Clamp(scroll_left, 0.0f, max_scroll) : scroll_left);
	if (new_offset != scroll_offset.x)
	{
		scroll_offset.x = new_offset;
		Scroll().UpdateScrollbar(ElementScroll::HORIZONTAL);
		DirtyAbsoluteOffset();

		DispatchEvent(EventId::Scroll, Dictionary());
	}
}
float Element::GetScrollTop()
{
	return scroll_offset.y;
}
void Element::SetScrollTop(float scroll_top, bool clamp)
{
	const float max_scroll = Math::Max(0.0f, GetScrollHeight() - GetClientHeight());
	const float new_offset = Math::Round(clamp ? Math::Clamp(scroll_top, 0.0f, max_scroll) : scroll_top);
	if (new_offset != scroll_offset.y)
	{
		scroll_offset.y = new_offset;
		Scroll().UpdateScrollbar(ElementScroll::VERTICAL);
		DirtyAbsoluteOffset();

		DispatchEvent(EventId::Scroll, Dictionary());
	}
}
float Element::GetScrollWidth()
{
	return Math::Max(scrollable_overflow_rectangle.x, GetClientWidth());
}
float Element::GetScrollHeight()
{
	return Math::Max(scrollable_overflow_rectangle.y, GetClientHeight());
}
void Element::ScrollIntoView(const ScrollIntoViewOptions options)
{
	const Vector2f size = main_box.GetSize(BoxArea::Border);
	ScrollBehavior scroll_behavior = options.behavior;

	for (Element* scroll_parent = parent; scroll_parent; scroll_parent = scroll_parent->GetParentNode())
	{
		using Style::Overflow;
		const ComputedValues& computed = scroll_parent->GetComputedValues();
		const bool scrollable_box_x = (computed.overflow_x() != Overflow::Visible && computed.overflow_x() != Overflow::Hidden);
		const bool scrollable_box_y = (computed.overflow_y() != Overflow::Visible && computed.overflow_y() != Overflow::Hidden);

		const Vector2f parent_scroll_size = {scroll_parent->GetScrollWidth(), scroll_parent->GetScrollHeight()};
		const Vector2f parent_client_size = {scroll_parent->GetClientWidth(), scroll_parent->GetClientHeight()};

		if ((scrollable_box_x && parent_scroll_size.x > parent_client_size.x) || (scrollable_box_y && parent_scroll_size.y > parent_client_size.y))
		{
			const Vector2f relative_offset = scroll_parent->GetAbsoluteOffset(BoxArea::Border) - GetAbsoluteOffset(BoxArea::Border);

			const Vector2f old_scroll_offset = {scroll_parent->GetScrollLeft(), scroll_parent->GetScrollTop()};
			const Vector2f parent_client_offset = {scroll_parent->GetClientLeft(), scroll_parent->GetClientTop()};

			const Vector2f delta_scroll_offset_start = parent_client_offset - relative_offset;
			const Vector2f delta_scroll_offset_end = delta_scroll_offset_start + size - parent_client_size;

			Vector2f scroll_delta = {
				scrollable_box_x ? GetScrollOffsetDelta(options.horizontal, delta_scroll_offset_start.x, delta_scroll_offset_end.x) : 0.f,
				scrollable_box_y ? GetScrollOffsetDelta(options.vertical, delta_scroll_offset_start.y, delta_scroll_offset_end.y) : 0.f,
			};

			scroll_parent->ScrollTo(old_scroll_offset + scroll_delta, scroll_behavior);

			// Currently, only a single scrollable parent can be smooth scrolled at a time, so any other parents must be instant scrolled.
			scroll_behavior = ScrollBehavior::Instant;
		}

		if ((scrollable_box_x || scrollable_box_y) && options.parentage == ScrollParentage::Closest)
			break;
	}
}
void Element::ScrollIntoView(bool align_with_top)
{
	ScrollIntoViewOptions options;
	options.vertical = (align_with_top ? ScrollAlignment::Start : ScrollAlignment::End);
	options.horizontal = ScrollAlignment::Nearest;
	ScrollIntoView(options);
}
void Element::ScrollTo(Vector2f offset, ScrollBehavior behavior)
{
	if (behavior != ScrollBehavior::Instant)
	{
		if (Context* context = GetContext())
		{
			context->PerformSmoothscrollOnTarget(this, offset - scroll_offset, behavior);
			return;
		}
	}

	SetScrollLeft(offset.x);
	SetScrollTop(offset.y);
}
Element* Element::GetClosestScrollableContainer()
{
	using namespace Style;

	Overflow overflow_x = meta->computed_values.overflow_x();
	Overflow overflow_y = meta->computed_values.overflow_y();
	bool scrollable_x = (overflow_x == Overflow::Auto || overflow_x == Overflow::Scroll);
	bool scrollable_y = (overflow_y == Overflow::Auto || overflow_y == Overflow::Scroll);

	scrollable_x = (scrollable_x && GetScrollWidth() > GetClientWidth());
	scrollable_y = (scrollable_y && GetScrollHeight() > GetClientHeight());

	if (scrollable_x || scrollable_y || meta->computed_values.overscroll_behavior() == OverscrollBehavior::Contain)
		return this;
	else if (parent)
		return parent->GetClosestScrollableContainer();

	return nullptr;
}
void Element::ClampScrollOffset()
{
	const Vector2f new_scroll_offset = {
		Math::Round(Math::Clamp(scroll_offset.x, 0.0f, Math::Max(0.f, GetScrollWidth() - GetClientWidth()))),
		Math::Round(Math::Clamp(scroll_offset.y, 0.0f, Math::Max(0.f, GetScrollHeight() - GetClientHeight()))),
	};

	if (new_scroll_offset != scroll_offset)
	{
		scroll_offset = new_scroll_offset;
		DirtyAbsoluteOffset();
	}

	// At this point the scrollbars have been resolved, both in terms of size and visibility. Update their properties
	// now so that any visibility changes in particular are reflected immediately on the next render. Otherwise we risk
	// that the scrollbars renders a frame late, since changes to scrollbars can happen during layouting.
	Scroll().UpdateProperties();
}
void Element::ClampScrollOffsetRecursive()
{
	ClampScrollOffset();
	const int num_children = GetNumChildren();
	for (int i = 0; i < num_children; ++i)
		GetChild(i)->ClampScrollOffsetRecursive();
}

} // namespace ui
