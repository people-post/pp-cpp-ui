#include "FormattingContext.h"
#include <ui/style/ComputedValues.h>
#include <ui/layout/LayoutElement.h>
#include <ui/base/Profiling.h>
#include "BlockFormattingContext.h"
#include "FlexFormattingContext.h"
#include "LayoutBox.h"
#include "ReplacedFormattingContext.h"
#include "TableFormattingContext.h"

namespace ui {

UniquePtr<LayoutBox> FormattingContext::FormatIndependent(ContainerBox* parent_container, Element* element, const Box* override_initial_box,
	FormattingContextType backup_context)
{
	UI_ZoneScopedC(0xAFAFAF);
	using namespace Style;

	if (LayoutElement::IsReplaced(element))
		return ReplacedFormattingContext::Format(parent_container, element, override_initial_box);

	FormattingContextType type = backup_context;

	auto& computed = LayoutElement::GetComputedValues(element);
	const Display display = computed.display();
	if (display == Display::Flex || display == Display::InlineFlex)
	{
		type = FormattingContextType::Flex;
	}
	else if (display == Display::Table || display == Display::InlineTable)
	{
		type = FormattingContextType::Table;
	}
	else if (display == Display::InlineBlock || display == Display::FlowRoot || display == Display::TableCell || computed.float_() != Float::None ||
		computed.position() == Position::Absolute || computed.position() == Position::Fixed || computed.overflow_x() != Overflow::Visible ||
		computed.overflow_y() != Overflow::Visible || !LayoutElement::GetParentNode(element) || LayoutElement::GetDisplay(LayoutElement::GetParentNode(element)) == Display::Flex)
	{
		type = FormattingContextType::Block;
	}

	switch (type)
	{
	case FormattingContextType::Block: return BlockFormattingContext::Format(parent_container, element, override_initial_box);
	case FormattingContextType::Table: return TableFormattingContext::Format(parent_container, element, override_initial_box);
	case FormattingContextType::Flex: return FlexFormattingContext::Format(parent_container, element, override_initial_box);
	case FormattingContextType::None: break;
	}

	return nullptr;
}

} // namespace ui
