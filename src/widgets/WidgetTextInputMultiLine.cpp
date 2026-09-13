#include "WidgetTextInputMultiLine.h"
#include <ui/base/Dictionary.h>
#include <ui/dom/ElementText.h>
#include <algorithm>

namespace ui {

WidgetTextInputMultiLine::WidgetTextInputMultiLine(ElementFormControl* parent) : WidgetTextInput(parent) {}

WidgetTextInputMultiLine::~WidgetTextInputMultiLine() {}

void WidgetTextInputMultiLine::SanitizeValue(String& value)
{
	value.erase(std::remove_if(value.begin(), value.end(), [](char c) { return c == '\r' || c == '\t'; }), value.end());
}

void WidgetTextInputMultiLine::LineBreak() {}

} // namespace ui
