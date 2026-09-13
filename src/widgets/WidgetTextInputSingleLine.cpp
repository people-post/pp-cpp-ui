#include "WidgetTextInputSingleLine.h"
#include <ui/base/Dictionary.h>
#include <ui/text/ElementText.h>
#include <ui/widgets/ElementFormControl.h>
#include <algorithm>

namespace ui {

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

} // namespace ui
