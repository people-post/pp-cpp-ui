#pragma once

#include <ui/layout/Box.h>
#include <ui/base/Header.h>
#include <ui/paint/RenderBox.h>
#include <ui/base/Types.h>

namespace ui {

class Element;

/**
    Box-model part of an Element: offsets, clip area, and laid-out boxes.

    Phase 1: non-owning view over Element geometry APIs. Later phases may move
    storage here without changing the parts-first call style (`element->BoxModel()`).
 */

class UI_CORE_API ElementBox {
public:
	explicit ElementBox(Element* element) noexcept : element(element) {}

	Element* GetElement() const noexcept { return element; }

	void SetOffset(Vector2f offset, Element* offset_parent, bool offset_fixed = false);
	Vector2f GetRelativeOffset(BoxArea area = BoxArea::Content);
	Vector2f GetAbsoluteOffset(BoxArea area = BoxArea::Content);

	void SetClipArea(BoxArea clip_area);
	BoxArea GetClipArea() const;

	void SetScrollableOverflowRectangle(Vector2f scrollable_overflow_rectangle, bool clamp_scroll_offset);

	void SetBox(const Box& box);
	void AddBox(const Box& box, Vector2f offset);
	const Box& GetBox();
	const Box& GetBox(int index, Vector2f& offset);
	RenderBox GetRenderBox(BoxArea fill_area = BoxArea::Padding, int index = 0);
	int GetNumBoxes();

private:
	Element* element;
};

} // namespace ui
