#include <ui/base/TypeConverter.h>
#include <ui/base/Animation.h>
#include <ui/base/DecorationTypes.h>
#include <ui/base/Unit.h>

namespace ui {

bool TypeConverter<Unit, String>::Convert(const Unit& src, String& dest)
{
	switch (src)
	{
		// clang-format off
	case Unit::NUMBER:  dest = "";    return true;
	case Unit::PERCENT: dest = "%";   return true;

	case Unit::PX:      dest = "px";  return true;
	case Unit::DP:      dest = "dp";  return true;
	case Unit::VW:      dest = "vw";  return true;
	case Unit::VH:      dest = "vh";  return true;
	case Unit::X:       dest = "x";   return true;
	case Unit::EM:      dest = "em";  return true;
	case Unit::REM:     dest = "rem"; return true;

	case Unit::INCH:    dest = "in";  return true;
	case Unit::CM:      dest = "cm";  return true;
	case Unit::MM:      dest = "mm";  return true;
	case Unit::PT:      dest = "pt";  return true;
	case Unit::PC:      dest = "pc";  return true;

	case Unit::DEG:     dest = "deg"; return true;
	case Unit::RAD:     dest = "rad"; return true;
	// clang-format on
	default: break;
	}

	return false;
}

bool TypeConverter<TransitionList, TransitionList>::Convert(const TransitionList& src, TransitionList& dest)
{
	dest = src;
	return true;
}

bool TypeConverter<AnimationList, AnimationList>::Convert(const AnimationList& src, AnimationList& dest)
{
	dest = src;
	return true;
}

bool TypeConverter<AnimationList, String>::Convert(const AnimationList& src, String& dest)
{
	String tmp;
	for (size_t i = 0; i < src.size(); i++)
	{
		const Animation& a = src[i];
		if (TypeConverter<float, String>::Convert(a.duration, tmp))
			dest += tmp + "s ";
		dest += a.tween.to_string() + " ";
		if (a.delay > 0.0f && TypeConverter<float, String>::Convert(a.delay, tmp))
			dest += tmp + "s ";
		if (a.alternate)
			dest += "alternate ";
		if (a.paused)
			dest += "paused ";
		if (a.num_iterations == -1)
			dest += "infinite ";
		else if (TypeConverter<int, String>::Convert(a.num_iterations, tmp))
			dest += tmp + " ";
		dest += a.name;
		if (i != src.size() - 1)
			dest += ", ";
	}
	return true;
}

bool TypeConverter<FontEffectsPtr, FontEffectsPtr>::Convert(const FontEffectsPtr& src, FontEffectsPtr& dest)
{
	dest = src;
	return true;
}

bool TypeConverter<FontEffectsPtr, String>::Convert(const FontEffectsPtr& src, String& dest)
{
	if (!src || src->list.empty())
		dest = "none";
	else
		dest += src->value;
	return true;
}

bool TypeConverter<ColorStopList, ColorStopList>::Convert(const ColorStopList& src, ColorStopList& dest)
{
	dest = src;
	return true;
}

bool TypeConverter<ColorStopList, String>::Convert(const ColorStopList& src, String& dest)
{
	dest.clear();
	for (size_t i = 0; i < src.size(); i++)
	{
		const ColorStop& stop = src[i];
		dest += ToString(stop.color.ToNonPremultiplied());

		if (Any(stop.position.unit & Unit::NUMBER_LENGTH_PERCENT))
			dest += " " + ToString(stop.position.number) + ToString(stop.position.unit);

		if (i < src.size() - 1)
			dest += ", ";
	}
	return true;
}

bool TypeConverter<BoxShadowList, BoxShadowList>::Convert(const BoxShadowList& src, BoxShadowList& dest)
{
	dest = src;
	return true;
}

bool TypeConverter<BoxShadowList, String>::Convert(const BoxShadowList& src, String& dest)
{
	dest.clear();
	String temp, str_unit;
	for (size_t i = 0; i < src.size(); i++)
	{
		const BoxShadow& shadow = src[i];
		for (const NumericValue* value : {&shadow.offset_x, &shadow.offset_y, &shadow.blur_radius, &shadow.spread_distance})
		{
			if (TypeConverter<Unit, String>::Convert(value->unit, str_unit))
				temp += " " + ToString(value->number) + str_unit;
		}

		if (shadow.inset)
			temp += " inset";

		dest += ToString(shadow.color.ToNonPremultiplied()) + temp;

		if (i < src.size() - 1)
		{
			dest += ", ";
			temp.clear();
		}
	}
	return true;
}

bool TypeConverter<Colourb, String>::Convert(const Colourb& src, String& dest)
{
	if (src.alpha == 255)
		return FormatString(dest, "#%02hhx%02hhx%02hhx", src.red, src.green, src.blue) > 0;
	else
		return FormatString(dest, "#%02hhx%02hhx%02hhx%02hhx", src.red, src.green, src.blue, src.alpha) > 0;
}

} // namespace ui
