#include "InlineLevelBox.h"
#include <ui/base/SystemInterface.h>
#include <ui/style/ComputedValues.h>
#include <ui/layout/LayoutTextElement.h>
#include <ui/dom/ElementUtilities.h>
#include "dom/ListMarker.h"
#include "LayoutDetails.h"
#include "LayoutPools.h"

namespace ui {

void* InlineLevelBox::operator new(size_t size)
{
	return LayoutPools::AllocateLayoutChunk(size);
}

void InlineLevelBox::operator delete(void* chunk, size_t size)
{
	LayoutPools::DeallocateLayoutChunk(chunk, size);
}

InlineLevelBox::~InlineLevelBox() {}

void InlineLevelBox::SubmitElementOnLayout()
{
	element->OnLayout();
}

const FontMetrics& InlineLevelBox::GetFontMetrics() const
{
	return element->GetFontMetrics();
}

void InlineLevelBox::SetHeightAndVerticalAlignment(float _height_above_baseline, float _depth_below_baseline, const InlineLevelBox* parent)
{
	UI_ASSERT(parent);
	using Style::VerticalAlign;

	SetHeight(_height_above_baseline, _depth_below_baseline);

	const Style::VerticalAlign vertical_align = element->GetComputedValues().vertical_align();
	vertical_align_type = vertical_align.type;

	// Determine the offset from the parent baseline.
	float parent_baseline_offset = 0.f; // The anchor on the parent, as an offset from its baseline.
	float self_baseline_offset = 0.f;   // The offset from the parent anchor to our baseline.

	switch (vertical_align.type)
	{
	case VerticalAlign::Baseline: parent_baseline_offset = 0.f; break;
	case VerticalAlign::Length: parent_baseline_offset = -vertical_align.value; break;
	case VerticalAlign::Sub: parent_baseline_offset = (1.f / 5.f) * (float)parent->GetFontMetrics().size; break;
	case VerticalAlign::Super: parent_baseline_offset = (-1.f / 3.f) * (float)parent->GetFontMetrics().size; break;
	case VerticalAlign::TextTop:
		parent_baseline_offset = -parent->GetFontMetrics().ascent;
		self_baseline_offset = height_above_baseline;
		break;
	case VerticalAlign::TextBottom:
		parent_baseline_offset = parent->GetFontMetrics().descent;
		self_baseline_offset = -depth_below_baseline;
		break;
	case VerticalAlign::Middle:
		parent_baseline_offset = -0.5f * parent->GetFontMetrics().x_height;
		self_baseline_offset = 0.5f * (height_above_baseline - depth_below_baseline);
		break;
	case VerticalAlign::Top:
	case VerticalAlign::Center:
	case VerticalAlign::Bottom:
		// These are relative to the line box and handled later.
		break;
	}

	vertical_offset_from_parent = parent_baseline_offset + self_baseline_offset;
}

void InlineLevelBox::SetHeight(float _height_above_baseline, float _depth_below_baseline)
{
	height_above_baseline = _height_above_baseline;
	depth_below_baseline = _depth_below_baseline;
}

void InlineLevelBox::SetInlineBoxSpacing(float _spacing_left, float _spacing_right)
{
	spacing_left = _spacing_left;
	spacing_right = _spacing_right;
}

String InlineLevelBox::DebugDumpTree(int depth) const
{
	String value = String(depth * 2, ' ') + DebugDumpNameValue() + " | " + LayoutDetails::GetDebugElementName(GetElement()) + '\n';
	return value;
}

InlineLevelBox_Atomic::InlineLevelBox_Atomic(const InlineLevelBox* parent, Element* element, const Box& box) : InlineLevelBox(element), box(box)
{
	UI_ASSERT(parent && element);
	UI_ASSERT(box.GetSize().x >= 0.f && box.GetSize().y >= 0.f);

	const float outer_height = box.GetSizeAcross(BoxDirection::Vertical, BoxArea::Margin);

	const float descent = GetElement()->GetBaseline();
	const float ascent = outer_height - descent;
	SetHeightAndVerticalAlignment(ascent, descent, parent);
}

FragmentConstructor InlineLevelBox_Atomic::CreateFragment(InlineLayoutMode mode, float available_width, float right_spacing_width, bool /*first_box*/,
	LayoutOverflowHandle /*overflow_handle*/)
{
	const float outer_width = box.GetSizeAcross(BoxDirection::Horizontal, BoxArea::Margin);

	if (mode != InlineLayoutMode::WrapAny || outer_width + right_spacing_width <= available_width)
		return FragmentConstructor{FragmentType::SizedBox, outer_width, {}, {}};

	return {};
}

void InlineLevelBox_Atomic::Submit(const PlacedFragment& placed_fragment)
{
	const Vector2f margin_position = {placed_fragment.position.x, placed_fragment.position.y - GetHeightAboveBaseline()};
	const Vector2f margin_edge = {box.GetEdge(BoxArea::Margin, BoxEdge::Left), box.GetEdge(BoxArea::Margin, BoxEdge::Top)};
	const Vector2f border_position = margin_position + margin_edge;

	GetElement()->SetOffset(border_position, placed_fragment.offset_parent);
	GetElement()->SetBox(box);
	SubmitElementOnLayout();
}

InlineLevelBox_Text::InlineLevelBox_Text(LayoutTextElement* element) : InlineLevelBox(element->GetLayoutElement()) {}

FragmentConstructor InlineLevelBox_Text::CreateFragment(InlineLayoutMode mode, float available_width, float right_spacing_width, bool first_box,
	LayoutOverflowHandle in_overflow_handle)
{
	LayoutTextElement* text_element = GetTextElement();
	Element* layout_element = text_element->GetLayoutElement();

	const bool allow_empty = (mode == InlineLayoutMode::WrapAny);
	const bool decode_escape_characters = true;

	String line_contents;
	int line_begin = in_overflow_handle;
	int line_length = 0;
	float line_width = 0.f;
	bool overflow = !text_element->GenerateLine(line_contents, line_length, line_width, line_begin, available_width, right_spacing_width, first_box,
		decode_escape_characters, allow_empty);

	// FORK_WORKAROUND: prepend list marker — replace with list-style/::marker when available.
	if (first_box && line_begin == 0 && !line_contents.empty())
	{
		if (String marker = GetListItemMarker(layout_element->GetParentNode()); !marker.empty())
		{
			line_width += float(ElementUtilities::GetStringWidth(layout_element, marker));
			line_contents.insert(0, marker);
		}
	}

	if (overflow && line_contents.empty())
		// We couldn't fit anything on this line.
		return {};

	LayoutOverflowHandle out_overflow_handle = {};
	if (overflow)
		out_overflow_handle = line_begin + line_length;

	LayoutFragmentHandle fragment_handle = (LayoutFragmentHandle)fragments.size();
	fragments.push_back(std::move(line_contents));

	return FragmentConstructor{FragmentType::TextRun, line_width, fragment_handle, out_overflow_handle};
}

void InlineLevelBox_Text::Submit(const PlacedFragment& placed_fragment)
{
	UI_ASSERT((size_t)placed_fragment.handle < fragments.size());

	const int fragment_index = (int)placed_fragment.handle;
	const bool principal_box = (fragment_index == 0);

	LayoutTextElement* text_element = GetTextElement();
	Element* layout_element = text_element->GetLayoutElement();
	Vector2f line_offset;

	if (principal_box)
	{
		element_offset = placed_fragment.position;
		layout_element->SetOffset(placed_fragment.position, placed_fragment.offset_parent);
		text_element->ClearLines();
	}
	else
	{
		line_offset = placed_fragment.position - element_offset;
	}

	text_element->AddLine(line_offset, std::move(fragments[fragment_index]));
}

String InlineLevelBox_Text::DebugDumpNameValue() const
{
	return "InlineLevelBox_Text";
}

LayoutTextElement* InlineLevelBox_Text::GetTextElement()
{
	LayoutTextElement* text = GetElement()->GetAsLayoutTextElement();
	UI_ASSERT(text);
	return text;
}
} // namespace ui
