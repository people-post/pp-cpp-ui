// Element — scroll API façades (thin wrappers over Scroll() part).
#include <ui/dom/Element.h>
#include <ui/dom/ElementScroll.h>
#include <ui/dom/Context.h>

namespace ui {

float Element::GetScrollLeft()
{
	return Scroll().GetScrollLeft();
}
void Element::SetScrollLeft(float scroll_left, bool clamp)
{
	Scroll().SetScrollLeft(scroll_left, clamp);
}
float Element::GetScrollTop()
{
	return Scroll().GetScrollTop();
}
void Element::SetScrollTop(float scroll_top, bool clamp)
{
	Scroll().SetScrollTop(scroll_top, clamp);
}
float Element::GetScrollWidth()
{
	return Scroll().GetScrollWidth();
}
float Element::GetScrollHeight()
{
	return Scroll().GetScrollHeight();
}
void Element::ScrollIntoView(const ScrollIntoViewOptions options)
{
	Scroll().ScrollIntoView(options);
}
void Element::ScrollIntoView(bool align_with_top)
{
	Scroll().ScrollIntoView(align_with_top);
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
	Scroll().ScrollTo(offset, ScrollBehavior::Instant);
}
Element* Element::GetClosestScrollableContainer()
{
	return Scroll().GetClosestScrollableContainer();
}
void Element::ClampScrollOffset()
{
	Scroll().ClampScrollOffset();
}
void Element::ClampScrollOffsetRecursive()
{
	Scroll().ClampScrollOffsetRecursive();
}

} // namespace ui
