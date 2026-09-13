#pragma once

#include <ui/base/Types.h>

namespace ui {

class Element;

enum class LayoutScrollbarAxis { Vertical = 0, Horizontal = 1 };

/**
    Layout-owned façade for Element operations that would otherwise pull
    ElementScroll / ElementUtilities / private DOM helpers into every layout TU.

    Concrete DOM includes stay in LayoutElement.cpp (Phase 1 of LAYOUT_DOM_BRIDGE.md).
 */
namespace LayoutElement {

float GetScrollbarSize(Element* element, LayoutScrollbarAxis axis);
void EnableScrollbar(Element* element, LayoutScrollbarAxis axis, float element_width);
void DisableScrollbar(Element* element, LayoutScrollbarAxis axis);
void FormatScrollbars(Element* element);

float GetStringWidth(Element* element, StringView string);

/// FORK_WORKAROUND: list-style / ::marker are unavailable; see docs/LAYOUT_DOM_BRIDGE.md.
String GetListItemMarker(Element* list_item_element);

} // namespace LayoutElement

} // namespace ui
