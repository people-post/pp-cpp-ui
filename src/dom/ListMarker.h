#pragma once

#include <ui/base/Types.h>
namespace ui {

class Element;

// FORK_WORKAROUND: browsers use list-style / ::marker; pp-cpp-ui has neither. This injects
// marker text during inline layout (InlineLevelBox.cpp). Limitations: direct <li>text</li>
// only; no <li><p>…</p></li>; fixed • / "N. " styles. See docs/architecture/UI_UPSTREAM.md.

/// Returns the list marker prefix for text rendered as a direct child of an li element.
String GetListItemMarker(Element* list_item_element);

} // namespace ui
