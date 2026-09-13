#include <ui/dom/ElementScroll.h>
#include <ui/style/ComputedValues.h>
#include <ui/dom/Context.h>
#include <ui/dom/Element.h>
#include <ui/dom/ElementUtilities.h>
#include <ui/dom/Event.h>
#include <ui/dom/Factory.h>
#include "ElementMeta.h"
#include "layout/LayoutDetails.h"
#include "WidgetScroll.h"

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

ElementScroll::ElementScroll(Element* _element)
{
	element = _element;
	corner = nullptr;
}

ElementScroll::~ElementScroll() {}

void ElementScroll::Update()
{
	for (int i = 0; i < 2; i++)
	{
		if (scrollbars[i].widget != nullptr)
			scrollbars[i].widget->Update();
	}
}

void ElementScroll::EnableScrollbar(Orientation orientation, float element_width)
{
	if (!scrollbars[orientation].enabled)
	{
		CreateScrollbar(orientation);
		scrollbars[orientation].element->Style().SetProperty(PropertyId::Visibility, Property(Style::Visibility::Visible));
		scrollbars[orientation].enabled = true;
	}

	// Determine the size of the scrollbar.
	Box box;
	LayoutDetails::BuildBox(box, Vector2f(element_width, element_width), scrollbars[orientation].element);

	if (orientation == VERTICAL)
		scrollbars[orientation].size = box.GetSize(BoxArea::Margin).x;
	if (orientation == HORIZONTAL)
	{
		if (box.GetSize().y < 0)
			scrollbars[orientation].size = box.GetCumulativeEdge(BoxArea::Content, BoxEdge::Left) +
				box.GetCumulativeEdge(BoxArea::Content, BoxEdge::Right) +
				ResolveValue(scrollbars[orientation].element->GetComputedValues().height(), element_width);
		else
			scrollbars[orientation].size = box.GetSize(BoxArea::Margin).y;
	}
}

void ElementScroll::DisableScrollbar(Orientation orientation)
{
	if (scrollbars[orientation].enabled)
	{
		scrollbars[orientation].element->Style().SetProperty(PropertyId::Visibility, Property(Style::Visibility::Hidden));
		scrollbars[orientation].enabled = false;

		if (corner)
			corner->Style().SetProperty(PropertyId::Visibility, Property(Style::Visibility::Hidden));
	}
}

void ElementScroll::UpdateScrollbar(Orientation orientation)
{
	float bar_position;
	float traversable_track;
	if (orientation == VERTICAL)
	{
		bar_position = GetScrollTop();
		traversable_track = GetScrollHeight() - element->GetClientHeight();
	}
	else
	{
		bar_position = GetScrollLeft();
		traversable_track = GetScrollWidth() - element->GetClientWidth();
	}

	if (traversable_track <= 0)
		bar_position = 0;
	else
		bar_position /= traversable_track;

	if (scrollbars[orientation].widget != nullptr)
	{
		bar_position = Math::Clamp(bar_position, 0.0f, 1.0f);

		if (scrollbars[orientation].widget->GetBarPosition() != bar_position)
			scrollbars[orientation].widget->SetBarPosition(bar_position);
	}
}

Element* ElementScroll::GetScrollbar(Orientation orientation)
{
	return scrollbars[orientation].element;
}

float ElementScroll::GetScrollbarSize(Orientation orientation)
{
	if (!scrollbars[orientation].enabled)
		return 0;

	return scrollbars[orientation].size;
}

void ElementScroll::FormatScrollbars()
{
	const Box& element_box = element->GetBox();
	const Vector2f containing_block = element_box.GetSize(BoxArea::Padding);

	for (int i = 0; i < 2; i++)
	{
		if (!scrollbars[i].enabled)
			continue;

		if (i == VERTICAL)
		{
			scrollbars[i].widget->SetBarLength(element->GetClientHeight());
			scrollbars[i].widget->SetTrackLength(GetScrollHeight());

			float traversable_track = GetScrollHeight() - element->GetClientHeight();
			if (traversable_track > 0)
				scrollbars[i].widget->SetBarPosition(GetScrollTop() / traversable_track);
			else
				scrollbars[i].widget->SetBarPosition(0);
		}
		else
		{
			scrollbars[i].widget->SetBarLength(element->GetClientWidth());
			scrollbars[i].widget->SetTrackLength(GetScrollWidth());

			float traversable_track = GetScrollWidth() - element->GetClientWidth();
			if (traversable_track > 0)
				scrollbars[i].widget->SetBarPosition(GetScrollLeft() / traversable_track);
			else
				scrollbars[i].widget->SetBarPosition(0);
		}

		float slider_length = containing_block[1 - i];
		float user_scrollbar_margin = scrollbars[i].element->GetComputedValues().scrollbar_margin();
		float min_scrollbar_margin = GetScrollbarSize(i == VERTICAL ? HORIZONTAL : VERTICAL);
		slider_length -= Math::Max(user_scrollbar_margin, min_scrollbar_margin);

		scrollbars[i].widget->FormatElements(containing_block, slider_length);

		int variable_axis = i == VERTICAL ? 0 : 1;
		Vector2f offset = element_box.GetPosition(BoxArea::Padding);
		offset[variable_axis] += containing_block[variable_axis] -
			(scrollbars[i].element->GetBox().GetSize(BoxArea::Border)[variable_axis] +
				scrollbars[i].element->GetBox().GetEdge(BoxArea::Margin, i == VERTICAL ? BoxEdge::Right : BoxEdge::Bottom));
		// Add the top or left margin (as appropriate) onto the scrollbar's position.
		offset[1 - variable_axis] += scrollbars[i].element->GetBox().GetEdge(BoxArea::Margin, i == VERTICAL ? BoxEdge::Top : BoxEdge::Left);
		scrollbars[i].element->SetOffset(offset, element, true);
	}

	// Format the corner, if it is necessary.
	if (scrollbars[0].enabled && scrollbars[1].enabled)
	{
		CreateCorner();

		Box corner_box;
		LayoutDetails::BuildBox(corner_box, Vector2f(containing_block.x), corner);

		corner_box.SetContent(Vector2f(scrollbars[VERTICAL].size, scrollbars[HORIZONTAL].size));
		corner->SetBox(corner_box);
		corner->SetOffset(containing_block + element_box.GetPosition(BoxArea::Padding) -
				Vector2f(scrollbars[VERTICAL].size, scrollbars[HORIZONTAL].size) - corner_box.GetPosition(BoxArea::Margin),
			element, true);

		corner->Style().SetProperty(PropertyId::Visibility, Property(Style::Visibility::Visible));
	}
}

void ElementScroll::UpdateProperties()
{
	for (Element* scroll_element : {scrollbars[VERTICAL].element, scrollbars[HORIZONTAL].element, corner})
	{
		if (scroll_element)
			UpdateScrollElementProperties(scroll_element);
	}
}

bool ElementScroll::CreateScrollbar(Orientation orientation)
{
	if (scrollbars[orientation].element && scrollbars[orientation].widget)
		return true;

	ElementPtr scrollbar_element =
		Factory::InstanceElement(element, "*", orientation == VERTICAL ? "scrollbarvertical" : "scrollbarhorizontal", XMLAttributes());
	scrollbars[orientation].element = scrollbar_element.get();
	scrollbars[orientation].element->Style().SetProperty(PropertyId::Clip, Property(1, Unit::NUMBER));
	scrollbars[orientation].element->Style().SetProperty(PropertyId::Drag, Property(Style::Drag::Block));

	scrollbars[orientation].widget = MakeUnique<WidgetScroll>(scrollbars[orientation].element);
	scrollbars[orientation].widget->Initialise(orientation == VERTICAL ? WidgetScroll::VERTICAL : WidgetScroll::HORIZONTAL);

	Element* child = element->AppendChild(std::move(scrollbar_element), false);

	UpdateScrollElementProperties(child);

	return true;
}

bool ElementScroll::CreateCorner()
{
	if (corner != nullptr)
		return true;

	ElementPtr corner_element = Factory::InstanceElement(element, "*", "scrollbarcorner", XMLAttributes());
	corner = corner_element.get();
	corner->Style().SetProperty(PropertyId::Clip, Property(1, Unit::NUMBER));
	corner->Style().SetProperty(PropertyId::Drag, Property(Style::Drag::Block));

	Element* child = element->AppendChild(std::move(corner_element), false);
	UpdateScrollElementProperties(child);

	return true;
}

void ElementScroll::UpdateScrollElementProperties(Element* scroll_element)
{
	// The construction of scrollbars can occur during layouting, then we need some properties and computed values straight away.
	// In particular their size. Furthermore, updating these properties straight away avoids dirtying the document after layouting,
	// which may result in one less additional and unnecessary layouting procedure.

	Context* context = element->GetContext();

	const float dp_ratio = (context ? context->GetDensityIndependentPixelRatio() : 1.0f);
	const Vector2f vp_dimensions = (context ? Vector2f(context->GetDimensions()) : Vector2f(1.0f));
	scroll_element->Update(dp_ratio, vp_dimensions);
}

float ElementScroll::GetScrollLeft() const
{
	return element->scroll_offset.x;
}

void ElementScroll::SetScrollLeft(float scroll_left, bool clamp)
{
	const float max_scroll = Math::Max(0.0f, GetScrollWidth() - element->GetClientWidth());
	const float new_offset = Math::Round(clamp ? Math::Clamp(scroll_left, 0.0f, max_scroll) : scroll_left);
	if (new_offset != element->scroll_offset.x)
	{
		element->scroll_offset.x = new_offset;
		UpdateScrollbar(HORIZONTAL);
		element->DirtyAbsoluteOffset();
		element->DispatchEvent(EventId::Scroll, Dictionary());
	}
}

float ElementScroll::GetScrollTop() const
{
	return element->scroll_offset.y;
}

void ElementScroll::SetScrollTop(float scroll_top, bool clamp)
{
	const float max_scroll = Math::Max(0.0f, GetScrollHeight() - element->GetClientHeight());
	const float new_offset = Math::Round(clamp ? Math::Clamp(scroll_top, 0.0f, max_scroll) : scroll_top);
	if (new_offset != element->scroll_offset.y)
	{
		element->scroll_offset.y = new_offset;
		UpdateScrollbar(VERTICAL);
		element->DirtyAbsoluteOffset();
		element->DispatchEvent(EventId::Scroll, Dictionary());
	}
}

float ElementScroll::GetScrollWidth() const
{
	return Math::Max(element->scrollable_overflow_rectangle.x, element->GetClientWidth());
}

float ElementScroll::GetScrollHeight() const
{
	return Math::Max(element->scrollable_overflow_rectangle.y, element->GetClientHeight());
}

void ElementScroll::ScrollIntoView(ScrollIntoViewOptions options)
{
	const Vector2f size = element->main_box.GetSize(BoxArea::Border);
	ScrollBehavior scroll_behavior = options.behavior;

	for (Element* scroll_parent = element->parent; scroll_parent; scroll_parent = scroll_parent->GetParentNode())
	{
		using Style::Overflow;
		const ComputedValues& computed = scroll_parent->GetComputedValues();
		const bool scrollable_box_x = (computed.overflow_x() != Overflow::Visible && computed.overflow_x() != Overflow::Hidden);
		const bool scrollable_box_y = (computed.overflow_y() != Overflow::Visible && computed.overflow_y() != Overflow::Hidden);

		ElementScroll& parent_scroll = scroll_parent->Scroll();
		const Vector2f parent_scroll_size = {parent_scroll.GetScrollWidth(), parent_scroll.GetScrollHeight()};
		const Vector2f parent_client_size = {scroll_parent->GetClientWidth(), scroll_parent->GetClientHeight()};

		if ((scrollable_box_x && parent_scroll_size.x > parent_client_size.x) || (scrollable_box_y && parent_scroll_size.y > parent_client_size.y))
		{
			const Vector2f relative_offset =
				scroll_parent->BoxModel().GetAbsoluteOffset(BoxArea::Border) - element->BoxModel().GetAbsoluteOffset(BoxArea::Border);

			const Vector2f old_scroll_offset = {parent_scroll.GetScrollLeft(), parent_scroll.GetScrollTop()};
			const Vector2f parent_client_offset = {scroll_parent->GetClientLeft(), scroll_parent->GetClientTop()};

			const Vector2f delta_scroll_offset_start = parent_client_offset - relative_offset;
			const Vector2f delta_scroll_offset_end = delta_scroll_offset_start + size - parent_client_size;

			Vector2f scroll_delta = {
				scrollable_box_x ? GetScrollOffsetDelta(options.horizontal, delta_scroll_offset_start.x, delta_scroll_offset_end.x) : 0.f,
				scrollable_box_y ? GetScrollOffsetDelta(options.vertical, delta_scroll_offset_start.y, delta_scroll_offset_end.y) : 0.f,
			};

			// Prefer Element::ScrollTo so smooth scrolling can use Context friendship.
			scroll_parent->ScrollTo(old_scroll_offset + scroll_delta, scroll_behavior);

			// Currently, only a single scrollable parent can be smooth scrolled at a time, so any other parents must be instant scrolled.
			scroll_behavior = ScrollBehavior::Instant;
		}

		if ((scrollable_box_x || scrollable_box_y) && options.parentage == ScrollParentage::Closest)
			break;
	}
}

void ElementScroll::ScrollIntoView(bool align_with_top)
{
	ScrollIntoViewOptions options;
	options.vertical = (align_with_top ? ScrollAlignment::Start : ScrollAlignment::End);
	options.horizontal = ScrollAlignment::Nearest;
	ScrollIntoView(options);
}

void ElementScroll::ScrollTo(Vector2f offset, ScrollBehavior behavior)
{
	// Smooth scrolling requires Context friendship; Element::ScrollTo owns that path.
	(void)behavior;
	SetScrollLeft(offset.x);
	SetScrollTop(offset.y);
}

Element* ElementScroll::GetClosestScrollableContainer()
{
	using namespace Style;

	Overflow overflow_x = element->meta->computed_values.overflow_x();
	Overflow overflow_y = element->meta->computed_values.overflow_y();
	bool scrollable_x = (overflow_x == Overflow::Auto || overflow_x == Overflow::Scroll);
	bool scrollable_y = (overflow_y == Overflow::Auto || overflow_y == Overflow::Scroll);

	scrollable_x = (scrollable_x && GetScrollWidth() > element->GetClientWidth());
	scrollable_y = (scrollable_y && GetScrollHeight() > element->GetClientHeight());

	if (scrollable_x || scrollable_y || element->meta->computed_values.overscroll_behavior() == OverscrollBehavior::Contain)
		return element;
	else if (element->parent)
		return element->parent->Scroll().GetClosestScrollableContainer();

	return nullptr;
}

void ElementScroll::ClampScrollOffset()
{
	const Vector2f new_scroll_offset = {
		Math::Round(Math::Clamp(element->scroll_offset.x, 0.0f, Math::Max(0.f, GetScrollWidth() - element->GetClientWidth()))),
		Math::Round(Math::Clamp(element->scroll_offset.y, 0.0f, Math::Max(0.f, GetScrollHeight() - element->GetClientHeight()))),
	};

	if (new_scroll_offset != element->scroll_offset)
	{
		element->scroll_offset = new_scroll_offset;
		element->DirtyAbsoluteOffset();
	}

	// At this point the scrollbars have been resolved, both in terms of size and visibility. Update their properties
	// now so that any visibility changes in particular are reflected immediately on the next render. Otherwise we risk
	// that the scrollbars renders a frame late, since changes to scrollbars can happen during layouting.
	UpdateProperties();
}

void ElementScroll::ClampScrollOffsetRecursive()
{
	ClampScrollOffset();
	const int num_children = element->GetNumChildren();
	for (int i = 0; i < num_children; ++i)
		element->GetChild(i)->Scroll().ClampScrollOffsetRecursive();
}

ElementScroll::Scrollbar::Scrollbar() {}

ElementScroll::Scrollbar::~Scrollbar() {}

} // namespace ui
