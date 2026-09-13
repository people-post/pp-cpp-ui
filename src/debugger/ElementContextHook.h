#pragma once

#include <ui/dom/ElementDocument.h>
#include "ElementDebugDocument.h"

namespace ui {
namespace Debugger {

class DebuggerPlugin;

/**
    An element that the debugger uses to render into a foreign context.
 */

class ElementContextHook : public ElementDebugDocument {
public:
	UI_RTTI_DefineWithParent(ElementContextHook, ElementDebugDocument)

	ElementContextHook(const String& tag);
	virtual ~ElementContextHook();

	void Initialise(DebuggerPlugin* debugger);

	void OnRender() override;

private:
	DebuggerPlugin* debugger;
};

} // namespace Debugger
} // namespace ui
