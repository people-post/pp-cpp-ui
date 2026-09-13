// Element — geometry / box-model cache (definitions only).
#include <ui/dom/Element.h>
#include <ui/dom/ElementDocument.h>
#include <ui/dom/ElementScroll.h>
#include <ui/style/ComputedValues.h>
#include <ui/paint/RenderBox.h>
#include <ui/base/Math.h>
#include "ElementMeta.h"
#include "ElementStyle.h"

namespace ui {

void Element::SetOffset(Vector2f offset, Element* _offset_parent, bool _offset_fixed)
{
	_offset_fixed |= GetPosition() == Style::Position::Fixed;

	// If our offset has definitely changed, or any of our parenting has, then these are set and
	// updated based on our left / right / top / bottom properties.
	if (relative_offset_base != offset || offset_parent != _offset_parent || offset_fixed != _offset_fixed)
	{
		relative_offset_base = offset;
		offset_fixed = _offset_fixed;
		offset_parent = _offset_parent;
		UpdateOffset();
		DirtyAbsoluteOffset();
	}

	// Otherwise, our offset is updated in case left / right / top / bottom will have an impact on
	// our final position, and our children are dirtied if they do.
	else
	{
		const Vector2f old_base = relative_offset_base;
		const Vector2f old_position = relative_offset_position;

		UpdateOffset();

		if (old_base != relative_offset_base || old_position != relative_offset_position)
			DirtyAbsoluteOffset();
	}
}
Vector2f Element::GetRelativeOffset(BoxArea area)
{
	return relative_offset_base + relative_offset_position + GetBox().GetPosition(area);
}
Vector2f Element::GetAbsoluteOffset(BoxArea area)
{
	UpdateAbsoluteOffsetAndRenderBoxData();
	return area == BoxArea::Border ? absolute_offset : absolute_offset + GetBox().GetPosition(area);
}
void Element::UpdateAbsoluteOffsetAndRenderBoxData()
{
	if (absolute_offset_dirty || rounded_main_padding_size_dirty)
	{
		absolute_offset_dirty = false;
		rounded_main_padding_size_dirty = false;

		Vector2f offset_from_ancestors;
		if (offset_parent)
			offset_from_ancestors = offset_parent->GetAbsoluteOffset(BoxArea::Border);

		if (!offset_fixed)
		{
			// Add any parent scrolling onto our position as well.
			if (offset_parent)
				offset_from_ancestors -= offset_parent->scroll_offset;

			// Finally, there may be relatively positioned elements between ourself and our containing block, add their relative offsets as well.
			for (Element* ancestor = parent; ancestor && ancestor != offset_parent; ancestor = ancestor->parent)
				offset_from_ancestors += ancestor->relative_offset_position;
		}

		// Fork: sticky clamp is recomputed whenever absolute offsets are dirtied (including on scroll).
		if (meta->computed_values.position() == Style::Position::Sticky)
		{
			const Vector2f unstuck_absolute = relative_offset_base + offset_from_ancestors;
			relative_offset_position = ComputeStickyOffset(unstuck_absolute);
		}

		const Vector2f relative_offset = relative_offset_base + relative_offset_position;
		absolute_offset = relative_offset + offset_from_ancestors;

		// Next, we find the rounded size of the box so that elements can be placed border-to-border next to each other
		// without any gaps. To achieve this, we have to adjust their rounded/rendered sizes based on their position, in
		// such a way that the bottom-right of this element exactly matches the top-left of the next element. The order
		// of floating-point operations matter here, we want to replicate the operations in the layout engine as close
		// as possible to avoid any gaps.
		const Vector2f main_padding_size = main_box.GetSize(BoxArea::Padding);
		const Vector2f bottom_right_absolute_offset = (relative_offset + main_padding_size) + offset_from_ancestors;
		const Vector2f new_rounded_main_padding_size = bottom_right_absolute_offset.Round() - absolute_offset.Round();
		if (new_rounded_main_padding_size != rounded_main_padding_size)
		{
			rounded_main_padding_size = new_rounded_main_padding_size;
			BackgroundBorder().DirtyBackground();
			BackgroundBorder().DirtyBorder();
			Effects().DirtyEffectsData();
		}
	}
}
void Element::SetClipArea(BoxArea _clip_area)
{
	clip_area = _clip_area;
}
BoxArea Element::GetClipArea() const
{
	return clip_area;
}
void Element::SetScrollableOverflowRectangle(Vector2f _scrollable_overflow_rectangle, bool clamp_scroll_offset)
{
	if (scrollable_overflow_rectangle != _scrollable_overflow_rectangle)
	{
		scrollable_overflow_rectangle = _scrollable_overflow_rectangle;
		if (clamp_scroll_offset)
			ClampScrollOffset();
	}
}
void Element::SetBox(const Box& box)
{
	if (box != main_box || additional_boxes.size() > 0)
	{
#ifdef UI_DEBUG
		for (const BoxEdge edge : {BoxEdge::Top, BoxEdge::Right, BoxEdge::Bottom, BoxEdge::Left})
		{
			const float border_width = box.GetEdge(BoxArea::Border, edge);
			if (border_width != Math::Round(border_width))
				Log::Message(Log::LT_WARNING, "Expected integer border width but got %g px on element: %s", border_width, GetAddress().c_str());
		}
#endif

		main_box = box;
		additional_boxes.clear();

		OnResize();
		rounded_main_padding_size_dirty = true;
		BackgroundBorder().DirtyBackground();
		BackgroundBorder().DirtyBorder();
		Effects().DirtyEffectsData();
	}
}
void Element::AddBox(const Box& box, Vector2f offset)
{
	additional_boxes.emplace_back(PositionedBox{box, offset});
	OnResize();
	BackgroundBorder().DirtyBackground();
	BackgroundBorder().DirtyBorder();
	Effects().DirtyEffectsData();
}
const Box& Element::GetBox()
{
	return main_box;
}
const Box& Element::GetBox(int index, Vector2f& offset)
{
	offset = Vector2f(0);

	const int additional_box_index = index - 1;
	if (index < 1 || additional_box_index >= (int)additional_boxes.size())
		return main_box;

	offset = additional_boxes[additional_box_index].offset;
	return additional_boxes[additional_box_index].box;
}
RenderBox Element::GetRenderBox(BoxArea fill_area, int index)
{
	UI_ASSERTMSG(fill_area >= BoxArea::Border && fill_area <= BoxArea::Content,
		"Render box can only be generated with fill area of border, padding or content.");

	UpdateAbsoluteOffsetAndRenderBoxData();

	struct BoxReference {
		const Box& box;
		Vector2f padding_size;
		Vector2f offset;
	};
	auto GetBoxAndOffset = [this, index]() {
		const int additional_box_index = index - 1;
		if (index < 1 || additional_box_index >= (int)additional_boxes.size())
			return BoxReference{main_box, rounded_main_padding_size, {}};
		const PositionedBox& positioned_box = additional_boxes[additional_box_index];
		return BoxReference{positioned_box.box, positioned_box.box.GetSize(BoxArea::Padding), positioned_box.offset.Round()};
	};

	BoxReference box = GetBoxAndOffset();

	EdgeSizes edge_sizes = {};
	for (int area = (int)BoxArea::Border; area < (int)fill_area; area++)
	{
		edge_sizes[0] += box.box.GetEdge(BoxArea(area), BoxEdge::Top);
		edge_sizes[1] += box.box.GetEdge(BoxArea(area), BoxEdge::Right);
		edge_sizes[2] += box.box.GetEdge(BoxArea(area), BoxEdge::Bottom);
		edge_sizes[3] += box.box.GetEdge(BoxArea(area), BoxEdge::Left);
	}
	Vector2f inner_size;
	switch (fill_area)
	{
	case BoxArea::Border: inner_size = box.padding_size + box.box.GetFrameSize(BoxArea::Border); break;
	case BoxArea::Padding: inner_size = box.padding_size; break;
	case BoxArea::Content: inner_size = box.padding_size - box.box.GetFrameSize(BoxArea::Padding); break;
	case BoxArea::Margin:
	case BoxArea::Auto: UI_ERROR;
	}

	return RenderBox{inner_size, box.offset, edge_sizes, meta->computed_values.border_radius()};
}
int Element::GetNumBoxes()
{
	return 1 + (int)additional_boxes.size();
}
float Element::GetBaseline() const
{
	return baseline;
}
bool Element::IsPointWithinElement(const Vector2f point)
{
	const Vector2f position = GetAbsoluteOffset(BoxArea::Border);

	for (int i = 0; i < GetNumBoxes(); ++i)
	{
		Vector2f box_offset;
		const Box& box = GetBox(i, box_offset);

		const Vector2f box_position = position + box_offset;
		const Vector2f box_dimensions = box.GetSize(BoxArea::Border);
		if (point.x >= box_position.x && point.x <= (box_position.x + box_dimensions.x) && point.y >= box_position.y &&
			point.y <= (box_position.y + box_dimensions.y))
		{
			return true;
		}
	}

	return false;
}
float Element::GetAbsoluteLeft()
{
	return GetAbsoluteOffset(BoxArea::Border).x;
}
float Element::GetAbsoluteTop()
{
	return GetAbsoluteOffset(BoxArea::Border).y;
}
float Element::GetClientLeft()
{
	return GetBox().GetPosition(BoxArea::Padding).x;
}
float Element::GetClientTop()
{
	return GetBox().GetPosition(BoxArea::Padding).y;
}
float Element::GetClientWidth()
{
	return GetBox().GetSize(BoxArea::Padding).x - Scroll().GetScrollbarSize(ElementScroll::VERTICAL);
}
float Element::GetClientHeight()
{
	return GetBox().GetSize(BoxArea::Padding).y - Scroll().GetScrollbarSize(ElementScroll::HORIZONTAL);
}
Element* Element::GetOffsetParent()
{
	return offset_parent;
}
float Element::GetOffsetLeft()
{
	return relative_offset_base.x + relative_offset_position.x;
}
float Element::GetOffsetTop()
{
	return relative_offset_base.y + relative_offset_position.y;
}
float Element::GetOffsetWidth()
{
	return GetBox().GetSize(BoxArea::Border).x;
}
float Element::GetOffsetHeight()
{
	return GetBox().GetSize(BoxArea::Border).y;
}
void Element::UpdateOffset()
{
	using namespace Style;
	const auto& computed = meta->computed_values;
	Position position_property = computed.position();

	if (position_property == Position::Absolute || position_property == Position::Fixed)
	{
		if (offset_parent != nullptr)
		{
			const Box& parent_box = offset_parent->GetBox();
			Vector2f containing_block = parent_box.GetSize(BoxArea::Padding);

			// If the element is anchored left, then the position is offset by that resolved value.
			if (computed.left().type != Left::Auto)
				relative_offset_base.x = parent_box.GetEdge(BoxArea::Border, BoxEdge::Left) +
					(ResolveValue(computed.left(), containing_block.x) + GetBox().GetEdge(BoxArea::Margin, BoxEdge::Left));

			// If the element is anchored right, then the position is set first so the element's right-most edge
			// (including margins) will render up against the containing box's right-most content edge, and then
			// offset by the resolved value.
			else if (computed.right().type != Right::Auto)
			{
				relative_offset_base.x = containing_block.x + parent_box.GetEdge(BoxArea::Border, BoxEdge::Left) -
					(ResolveValue(computed.right(), containing_block.x) + GetBox().GetSize(BoxArea::Border).x +
						GetBox().GetEdge(BoxArea::Margin, BoxEdge::Right));
			}

			// If the element is anchored top, then the position is offset by that resolved value.
			if (computed.top().type != Top::Auto)
			{
				relative_offset_base.y = parent_box.GetEdge(BoxArea::Border, BoxEdge::Top) +
					(ResolveValue(computed.top(), containing_block.y) + GetBox().GetEdge(BoxArea::Margin, BoxEdge::Top));
			}

			// If the element is anchored bottom, then the position is set first so the element's right-most edge
			// (including margins) will render up against the containing box's right-most content edge, and then
			// offset by the resolved value.
			else if (computed.bottom().type != Bottom::Auto)
			{
				relative_offset_base.y = containing_block.y + parent_box.GetEdge(BoxArea::Border, BoxEdge::Top) -
					(ResolveValue(computed.bottom(), containing_block.y) + GetBox().GetSize(BoxArea::Border).y +
						GetBox().GetEdge(BoxArea::Margin, BoxEdge::Bottom));
			}
		}
	}
	else if (position_property == Position::Relative)
	{
		if (offset_parent != nullptr)
		{
			const Box& parent_box = offset_parent->GetBox();
			Vector2f containing_block = parent_box.GetSize();

			if (computed.left().type != Left::Auto)
				relative_offset_position.x = ResolveValue(computed.left(), containing_block.x);
			else if (computed.right().type != Right::Auto)
				relative_offset_position.x = -1 * ResolveValue(computed.right(), containing_block.x);
			else
				relative_offset_position.x = 0;

			if (computed.top().type != Top::Auto)
				relative_offset_position.y = ResolveValue(computed.top(), containing_block.y);
			else if (computed.bottom().type != Bottom::Auto)
				relative_offset_position.y = -1 * ResolveValue(computed.bottom(), containing_block.y);
			else
				relative_offset_position.y = 0;
		}
	}
	else if (position_property == Position::Sticky)
	{
		// Sticky offsets depend on scroll; zero until absolute offsets are resolved (or recompute eagerly).
		relative_offset_position = {};
		if (!absolute_offset_dirty)
		{
			// Layout just finished; seed sticky from current scroll using unstuck flow position.
			Vector2f offset_from_ancestors;
			if (offset_parent)
			{
				offset_from_ancestors = offset_parent->GetAbsoluteOffset(BoxArea::Border);
				if (!offset_fixed)
					offset_from_ancestors -= offset_parent->scroll_offset;
			}
			for (Element* ancestor = parent; ancestor && ancestor != offset_parent; ancestor = ancestor->parent)
				offset_from_ancestors += ancestor->relative_offset_position;
			relative_offset_position = ComputeStickyOffset(relative_offset_base + offset_from_ancestors);
		}
	}
	else
	{
		relative_offset_position.x = 0;
		relative_offset_position.y = 0;
	}
}
Vector2f Element::ComputeStickyOffset(Vector2f unstuck_absolute_border)
{
	using namespace Style;

	// Nearest scrollport ancestor (overflow not visible on either axis), else the document.
	Element* scroll_container = nullptr;
	for (Element* ancestor = parent; ancestor; ancestor = ancestor->parent)
	{
		const auto& ac = ancestor->GetComputedValues();
		if (ac.overflow_x() != Overflow::Visible || ac.overflow_y() != Overflow::Visible)
		{
			scroll_container = ancestor;
			break;
		}
	}
	if (!scroll_container)
		scroll_container = owner_document;
	if (!scroll_container)
		return {};

	const auto& computed = meta->computed_values;
	const Vector2f size = GetBox().GetSize(BoxArea::Border);

	// Visible scrollport (padding box client area) in absolute coordinates.
	const Vector2f scroll_abs = scroll_container->GetAbsoluteOffset(BoxArea::Border);
	const Box& scroll_box = scroll_container->GetBox();
	const Vector2f scrollport_tl = scroll_abs + scroll_box.GetPosition(BoxArea::Padding);
	const Vector2f scrollport_size = {scroll_container->GetClientWidth(), scroll_container->GetClientHeight()};

	// Sticky constraint rectangle: parent's padding box (element cannot escape its parent).
	Element* constraint = parent ? parent : scroll_container;
	const Vector2f constraint_abs = constraint->GetAbsoluteOffset(BoxArea::Border);
	const Box& constraint_box = constraint->GetBox();
	const Vector2f constraint_tl = constraint_abs + constraint_box.GetPosition(BoxArea::Padding);
	const Vector2f constraint_size = constraint_box.GetSize(BoxArea::Padding);

	const Vector2f containing_block = GetContainingBlock();
	Vector2f sticky_offset;

	const bool has_top = computed.top().type != Top::Auto;
	const bool has_bottom = computed.bottom().type != Bottom::Auto;
	const bool has_left = computed.left().type != Left::Auto;
	const bool has_right = computed.right().type != Right::Auto;

	if (has_top || has_bottom)
	{
		float y = 0.f;
		if (has_top)
		{
			const float top_inset = ResolveValue(computed.top(), containing_block.y);
			const float desired = scrollport_tl.y + top_inset;
			if (unstuck_absolute_border.y < desired)
				y = desired - unstuck_absolute_border.y;
		}
		else // has_bottom
		{
			const float bottom_inset = ResolveValue(computed.bottom(), containing_block.y);
			const float desired_bottom = scrollport_tl.y + scrollport_size.y - bottom_inset;
			const float unstuck_bottom = unstuck_absolute_border.y + size.y;
			if (unstuck_bottom > desired_bottom)
				y = desired_bottom - unstuck_bottom;
		}

		const float min_y = constraint_tl.y - unstuck_absolute_border.y;
		const float max_y = (constraint_tl.y + constraint_size.y - size.y) - unstuck_absolute_border.y;
		sticky_offset.y = Math::Clamp(y, Math::Min(min_y, max_y), Math::Max(min_y, max_y));
	}

	if (has_left || has_right)
	{
		float x = 0.f;
		if (has_left)
		{
			const float left_inset = ResolveValue(computed.left(), containing_block.x);
			const float desired = scrollport_tl.x + left_inset;
			if (unstuck_absolute_border.x < desired)
				x = desired - unstuck_absolute_border.x;
		}
		else // has_right
		{
			const float right_inset = ResolveValue(computed.right(), containing_block.x);
			const float desired_right = scrollport_tl.x + scrollport_size.x - right_inset;
			const float unstuck_right = unstuck_absolute_border.x + size.x;
			if (unstuck_right > desired_right)
				x = desired_right - unstuck_right;
		}

		const float min_x = constraint_tl.x - unstuck_absolute_border.x;
		const float max_x = (constraint_tl.x + constraint_size.x - size.x) - unstuck_absolute_border.x;
		sticky_offset.x = Math::Clamp(x, Math::Min(min_x, max_x), Math::Max(min_x, max_x));
	}

	return sticky_offset;
}
void Element::SetBaseline(float in_baseline)
{
	baseline = in_baseline;
}

} // namespace ui
