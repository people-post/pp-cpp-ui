#include <ui/style/Property.h>
#include <ui/style/PropertyDefinition.h>
namespace ui {

Property::Property() : unit(Unit::UNKNOWN), specificity(-1)
{
	definition = nullptr;
	parser_index = -1;
}

String Property::ToString() const
{
	if (!definition)
		return value.Get<String>() + ui::ToString(unit);

	String string;
	definition->GetValue(string, *this);
	return string;
}

NumericValue Property::GetNumericValue() const
{
	NumericValue result;
	if (Any(unit & Unit::NUMERIC))
	{
		if (value.GetInto(result.number))
			result.unit = unit;
	}
	return result;
}

} // namespace ui
