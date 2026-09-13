// Element — local stacking context (definitions only).
#include <ui/dom/Element.h>
#include <ui/style/ComputedValues.h>
#include <algorithm>

namespace ui {

enum class RenderOrder {
	StackNegative, // Local stacking context with z < 0.
	Block,
	TableColumnGroup,
	TableColumn,
	TableRowGroup,
	TableRow,
	TableCell,
	Floating,
	Inline,
	Positioned,    // Positioned element, or local stacking context with z == 0.
	StackPositive, // Local stacking context with z > 0.
};
struct StackingContextChild {
	Element* element = nullptr;
	RenderOrder order = {};
};
static bool operator<(const StackingContextChild& lhs, const StackingContextChild& rhs)
{
	if (int(lhs.order) == int(rhs.order))
		return lhs.element->GetZIndex() < rhs.element->GetZIndex();
	return int(lhs.order) < int(rhs.order);
}

// Treat all children in the range [index_begin, end) as if the parent created a new stacking context, by sorting them
// separately and then assigning their parent's paint order. However, positioned and descendants which create a new
// stacking context should be considered part of the parent stacking context. See CSS 2, Appendix E.
static void StackingContext_MakeAtomicRange(Vector<StackingContextChild>& stacking_children, size_t index_begin, RenderOrder parent_render_order)
{
	std::stable_sort(stacking_children.begin() + index_begin, stacking_children.end());

	for (auto it = stacking_children.begin() + index_begin; it != stacking_children.end(); ++it)
	{
		auto order = it->order;
		if (order != RenderOrder::StackNegative && order != RenderOrder::Positioned && order != RenderOrder::StackPositive)
			it->order = parent_render_order;
	}
}


void Element::ForceLocalStackingContext()
{
	local_stacking_context_forced = true;
	local_stacking_context = true;

	DirtyStackingContext();
}

void Element::BuildLocalStackingContext()
{
	stacking_context_dirty = false;

	Vector<StackingContextChild> stacking_children;
	AddChildrenToStackingContext(stacking_children);
	std::stable_sort(stacking_children.begin(), stacking_children.end());

	ElementList& local_stacking = EnsureStackingContext();
	local_stacking.resize(stacking_children.size());
	for (size_t i = 0; i < stacking_children.size(); i++)
		local_stacking[i] = stacking_children[i].element;
}

void Element::AddChildrenToStackingContext(Vector<StackingContextChild>& stacking_children)
{
	bool is_flex_container = (GetDisplay() == Style::Display::Flex);
	const int num_children = (int)children.size();
	for (int i = 0; i < num_children; ++i)
	{
		const bool is_non_dom_element = (i >= num_children - num_non_dom_children);
		children[i]->AddToStackingContext(stacking_children, is_flex_container, is_non_dom_element);
	}
}

void Element::AddToStackingContext(Vector<StackingContextChild>& stacking_children, bool is_flex_item, bool is_non_dom_element)
{
	using Style::Display;

	if (!IsVisible())
		return;

	const Display display = GetDisplay();

	RenderOrder order = RenderOrder::Inline;
	bool include_children = true;
	bool render_as_atomic_unit = false;

	if (local_stacking_context)
	{
		if (z_index > 0.f)
			order = RenderOrder::StackPositive;
		else if (z_index < 0.f)
			order = RenderOrder::StackNegative;
		else
			order = RenderOrder::Positioned;

		include_children = false;
	}
	else if (display == Display::TableRow || display == Display::TableRowGroup || display == Display::TableColumn ||
		display == Display::TableColumnGroup)
	{
		// Handle internal display values taking priority over position and float.
		switch (display)
		{
		case Display::TableRow: order = RenderOrder::TableRow; break;
		case Display::TableRowGroup: order = RenderOrder::TableRowGroup; break;
		case Display::TableColumn: order = RenderOrder::TableColumn; break;
		case Display::TableColumnGroup: order = RenderOrder::TableColumnGroup; break;
		default: break;
		}
	}
	else if (GetPosition() != Style::Position::Static)
	{
		order = RenderOrder::Positioned;
		render_as_atomic_unit = true;
	}
	else if (GetFloat() != Style::Float::None)
	{
		order = RenderOrder::Floating;
		render_as_atomic_unit = true;
	}
	else
	{
		switch (display)
		{
		case Display::Block:
		case Display::FlowRoot:
		case Display::Table:
		case Display::Flex:
			order = RenderOrder::Block;
			render_as_atomic_unit = (display == Display::Table || is_flex_item);
			break;

		case Display::Inline:
		case Display::InlineBlock:
		case Display::InlineFlex:
		case Display::InlineTable:
			order = RenderOrder::Inline;
			render_as_atomic_unit = (display != Display::Inline || is_flex_item);
			break;

		case Display::TableCell:
			order = RenderOrder::TableCell;
			render_as_atomic_unit = true;
			break;

		case Display::TableRow:
		case Display::TableRowGroup:
		case Display::TableColumn:
		case Display::TableColumnGroup:
		case Display::None: UI_ERROR; break; // Handled above.
		}
	}

	if (is_non_dom_element)
		render_as_atomic_unit = true;

	stacking_children.push_back(StackingContextChild{this, order});

	if (include_children && !children.empty())
	{
		const size_t index_child_begin = stacking_children.size();

		AddChildrenToStackingContext(stacking_children);

		if (render_as_atomic_unit)
			StackingContext_MakeAtomicRange(stacking_children, index_child_begin, order);
	}
}

void Element::DirtyStackingContext()
{
	// Find the first ancestor that has a local stacking context, that is our stacking context parent.
	Element* stacking_context_parent = this;
	while (stacking_context_parent && !stacking_context_parent->local_stacking_context)
	{
		stacking_context_parent = stacking_context_parent->GetParentNode();
	}

	if (stacking_context_parent)
		stacking_context_parent->stacking_context_dirty = true;
}

} // namespace ui
