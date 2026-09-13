#include <ui/base/TypeConverter.h>
#include <ui/base/Animation.h>
#include <ui/style/Decorator.h>
#include <ui/style/Filter.h>
#include <ui/style/PropertyDictionary.h>
#include <ui/style/PropertySpecification.h>
#include <ui/style/StyleSheetSpecification.h>
#include <ui/style/StyleSheetTypes.h>
#include <ui/style/Transform.h>
#include <ui/style/TransformPrimitive.h>
#include "PropertyParserColour.h"
#include "PropertyParserDecorator.h"
#include "TransformUtilities.h"

namespace ui {

bool TypeConverter<TransformPtr, TransformPtr>::Convert(const TransformPtr& src, TransformPtr& dest)
{
	dest = src;
	return true;
}

bool TypeConverter<TransformPtr, String>::Convert(const TransformPtr& src, String& dest)
{
	if (src)
	{
		dest.clear();
		const Transform::PrimitiveList& primitives = src->GetPrimitives();
		for (size_t i = 0; i < primitives.size(); i++)
		{
			dest += TransformUtilities::ToString(primitives[i]);
			if (i != primitives.size() - 1)
				dest += ' ';
		}
	}
	else
	{
		dest = "none";
	}
	return true;
}

bool TypeConverter<TransitionList, String>::Convert(const TransitionList& src, String& dest)
{
	if (src.none)
	{
		dest = "none";
		return true;
	}
	String tmp;
	for (size_t i = 0; i < src.transitions.size(); i++)
	{
		const Transition& t = src.transitions[i];
		dest += StyleSheetSpecification::GetPropertyName(t.id) + ' ';
		dest += t.tween.to_string() + ' ';
		if (TypeConverter<float, String>::Convert(t.duration, tmp))
			dest += tmp + "s ";
		if (t.delay > 0.0f && TypeConverter<float, String>::Convert(t.delay, tmp))
			dest += tmp + "s ";
		if (t.reverse_adjustment_factor > 0.0f && TypeConverter<float, String>::Convert(t.reverse_adjustment_factor, tmp))
			dest += tmp + ' ';
		if (dest.size() > 0)
			dest.resize(dest.size() - 1);
		if (i != src.transitions.size() - 1)
			dest += ", ";
	}
	return true;
}

template <typename EffectDeclaration>
void AppendPaintArea(const EffectDeclaration& /*declaration*/, String& /*dest*/)
{}
template <>
void AppendPaintArea(const DecoratorDeclaration& declaration, String& dest)
{
	if (declaration.paint_area >= BoxArea::Border && declaration.paint_area <= BoxArea::Padding)
		dest += " " + PropertyParserDecorator::ConvertAreaToString(declaration.paint_area);
}

template <typename EffectsPtr>
static bool ConvertEffectToString(const EffectsPtr& src, String& dest, const String& separator)
{
	if (!src || src->list.empty())
		dest = "none";
	else if (!src->value.empty())
		dest += src->value;
	else
	{
		dest.clear();
		for (const auto& declaration : src->list)
		{
			dest += declaration.type;
			if (auto* instancer = declaration.instancer)
				dest += '(' + instancer->GetPropertySpecification().PropertiesToString(declaration.properties, false, ' ') + ')';

			AppendPaintArea(declaration, dest);
			if (&declaration != &src->list.back())
				dest += separator;
		}
	}
	return true;
}

bool TypeConverter<DecoratorsPtr, DecoratorsPtr>::Convert(const DecoratorsPtr& src, DecoratorsPtr& dest)
{
	dest = src;
	return true;
}

bool TypeConverter<DecoratorsPtr, String>::Convert(const DecoratorsPtr& src, String& dest)
{
	return ConvertEffectToString(src, dest, ", ");
}

bool TypeConverter<FiltersPtr, FiltersPtr>::Convert(const FiltersPtr& src, FiltersPtr& dest)
{
	dest = src;
	return true;
}

bool TypeConverter<FiltersPtr, String>::Convert(const FiltersPtr& src, String& dest)
{
	return ConvertEffectToString(src, dest, " ");
}

bool TypeConverter<String, Colourb>::Convert(const String& src, Colourb& dest)
{
	return PropertyParserColour::ParseColour(dest, src);
}


} // namespace ui
