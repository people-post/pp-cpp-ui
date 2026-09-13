#include "ElementDebugDocument.h"

namespace ui {
namespace Debugger {

ElementDebugDocument::ElementDebugDocument(const String& tag) : ElementDocument(tag)
{
	SetFocusableFromModal(true);
}

} // namespace Debugger
} // namespace ui
