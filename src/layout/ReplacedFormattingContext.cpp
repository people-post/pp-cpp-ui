#include "ReplacedFormattingContext.h"
#include <ui/style/ComputedValues.h>
#include "BlockFormattingContext.h"
#include "ContainerBox.h"
#include "LayoutDetails.h"
#include <ui/layout/LayoutElement.h>

namespace ui {

UniquePtr<LayoutBox> ReplacedFormattingContext::Format(ContainerBox* parent_container, Element* element, const Box* override_initial_box)
{
	UI_ASSERT(LayoutElement::IsReplaced(element));

	// Replaced elements provide their own rendering, we just set their box here and notify them that the element has been sized.
	auto replaced_box = MakeUnique<ReplacedBox>(element);
	Box& box = replaced_box->GetBox();
	if (override_initial_box)
		box = *override_initial_box;
	else
	{
		const Vector2f containing_block = LayoutDetails::GetContainingBlock(parent_container, LayoutElement::GetPosition(element)).size;
		LayoutDetails::BuildBox(box, containing_block, element);
	}

	// Submit the box and notify the element.
	replaced_box->Close();

	// Usually, replaced elements add children to the hidden DOM. If we happen to have any normal DOM children, e.g.
	// added by the user, we format them using normal block formatting rules. Since replaced elements provide their
	// own rendering, this could cause conflicting or strange layout results, and is done at the user's own risk.
	if (LayoutElement::HasChildNodes(element))
	{
		RootBox root(box);
		BlockFormattingContext::Format(&root, element, &box);
	}

	return replaced_box;
}

void ReplacedBox::Close()
{
	LayoutElement::SetBox(element, box);
	LayoutElement::OnLayout(element);
}

String ReplacedBox::DebugDumpTree(int depth) const
{
	return String(depth * 2, ' ') + "ReplacedBox";
}

} // namespace ui
