#include "PropertyParserString.h"

namespace ui {

PropertyParserString::PropertyParserString() {}

PropertyParserString::~PropertyParserString() {}

bool PropertyParserString::ParseValue(Property& property, const String& value, const ParameterMap& /*parameters*/) const
{
	property.value = Variant(value);
	property.unit = Unit::STRING;

	return true;
}

} // namespace ui
