#pragma once

#include <ui/base/FontMetrics.h>
#include <ui/base/Types.h>
#include <ui/layout/Box.h>
#include <ui/style/ComputedValues.h>
#include <ui/style/StyleTypes.h>

namespace ui {

class Element;
class LayoutTextElement;

enum class LayoutScrollbarAxis { Vertical = 0, Horizontal = 1 };

/**
    Layout-owned façade for Element operations that would otherwise pull DOM
    headers into every layout TU.

    Concrete DOM includes stay in LayoutElement.cpp — see docs/LAYOUT_DOM_BRIDGE.md.
 */
namespace LayoutElement {

// --- Phase 1: scroll / metrics / list marker ---
float GetScrollbarSize(Element* element, LayoutScrollbarAxis axis);
void EnableScrollbar(Element* element, LayoutScrollbarAxis axis, float element_width);
void DisableScrollbar(Element* element, LayoutScrollbarAxis axis);
void FormatScrollbars(Element* element);
float GetStringWidth(Element* element, StringView string);
String GetListItemMarker(Element* list_item_element);

// --- Phase 2: high-frequency layout ops ---
const ComputedValues& GetComputedValues(Element* element);
Style::Display GetDisplay(Element* element);
Style::Position GetPosition(Element* element);
float GetLineHeight(Element* element);
const FontMetrics& GetFontMetrics(Element* element);

const Box& GetBox(Element* element);
void SetBox(Element* element, const Box& box);
void AddBox(Element* element, const Box& box, Vector2f offset);
void SetOffset(Element* element, Vector2f offset, Element* offset_parent, bool offset_fixed = false);
void UpdateOffset(Element* element);
void SetBaseline(Element* element, float baseline);
float GetBaseline(Element* element);

void OnLayout(Element* element);
void ClampScrollOffsetRecursive(Element* element);
void SetScrollableOverflowRectangle(Element* element, Vector2f scrollable_overflow_rectangle, bool clamp_scroll_offset);

Element* GetParentNode(Element* element);
Element* GetChild(Element* element, int index);
int GetNumChildren(Element* element, bool include_non_dom_elements = false);
Element* GetOffsetParent(Element* element);
Vector2f GetRelativeOffset(Element* element, BoxArea area = BoxArea::Content);

bool IsReplaced(Element* element);
bool GetIntrinsicDimensions(Element* element, Vector2f& dimensions, float& ratio);

const String& GetId(Element* element);
const String& GetTagName(Element* element);
String GetAddress(Element* element, bool include_pseudo_classes = false, bool include_parents = true);
bool HasAttribute(Element* element, const String& name);
String GetAttributeString(Element* element, const String& name, const String& default_value = String());
int GetAttributeInt(Element* element, const String& name, int default_value);

LayoutTextElement* GetAsLayoutTextElement(Element* element);

} // namespace LayoutElement

} // namespace ui
