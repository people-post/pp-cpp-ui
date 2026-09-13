#include "WidgetTextInputSingleLine.h"
#include <ui/Core/Dictionary.h>
#include <ui/Core/ElementText.h>
#include <ui/Core/Elements/ElementFormControl.h>
#include <algorithm>

namespace Rml {

WidgetTextInputSingleLine::WidgetTextInputSingleLine(ElementFormControl* parent) : WidgetTextInput(parent)
{
	// Single line text controls should clip to the content area, see visual test: text_input_overflow.rml
	parent->SetClipArea(BoxArea::Content);
}

void WidgetTextInputSingleLine::SanitizeValue(String& value)
{
	value.erase(std::remove_if(value.begin(), value.end(), [](char c) { return c == '\r' || c == '\n' || c == '\t'; }), value.end());
}

void WidgetTextInputSingleLine::LineBreak()
{
	DispatchChangeEvent(true);
}

} // namespace Rml
