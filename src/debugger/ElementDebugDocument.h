#pragma once

#include <ui/Core/ElementDocument.h>

namespace ui {
namespace Debugger {

class ElementDebugDocument : public ElementDocument {
public:
	UI_RTTI_DefineWithParent(ElementDebugDocument, ElementDocument)

	ElementDebugDocument(const String& tag);
};

} // namespace Debugger
} // namespace ui
