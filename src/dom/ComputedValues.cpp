#include <ui/style/ComputedValues.h>
#include <ui/dom/Element.h>
#include "style/ComputeProperty.h"

namespace ui {

const AnimationList* Style::ComputedValues::animation() const
{
	if (auto p = element->GetLocalProperty(PropertyId::Animation))
	{
		if (p->unit == Unit::ANIMATION)
			return &(p->value.GetReference<AnimationList>());
	}
	return nullptr;
}

const TransitionList* Style::ComputedValues::transition() const
{
	if (auto p = element->GetLocalProperty(PropertyId::Transition))
	{
		if (p->unit == Unit::TRANSITION)
			return &(p->value.GetReference<TransitionList>());
	}
	return nullptr;
}

String Style::ComputedValues::font_family() const
{
	if (auto p = element->GetProperty(PropertyId::FontFamily))
		return ComputeFontFamily(p->Get<String>());

	return String();
}

String Style::ComputedValues::cursor() const
{
	if (auto p = element->GetProperty(PropertyId::Cursor))
		return p->Get<String>();

	return String();
}

float Style::ComputedValues::letter_spacing() const
{
	if (inherited.has_letter_spacing)
	{
		if (auto p = element->GetProperty(PropertyId::LetterSpacing))
			return element->ResolveLength(p->GetNumericValue());
	}
	return 0.f;
}

float ResolveValueOr(Style::LengthPercentageAuto length, float base_value, float default_value)
{
	if (length.type == Style::LengthPercentageAuto::Length)
		return length.value;
	// Percentages against a zero or indefinite containing block are undefined; treat as 'auto' (default_value).
	else if (length.type == Style::LengthPercentageAuto::Percentage && base_value > 0.f)
		return length.value * 0.01f * base_value;
	return default_value;
}

float ResolveValueOr(Style::LengthPercentage length, float base_value, float default_value)
{
	if (length.type == Style::LengthPercentage::Length)
		return length.value;
	else if (length.type == Style::LengthPercentage::Percentage && base_value > 0.f)
		return length.value * 0.01f * base_value;
	return default_value;
}


TransformPtr Style::ComputedValues::transform() const
{
	if (auto p = element->GetLocalProperty(PropertyId::Transform))
		return p->Get<TransformPtr>();
	return TransformPtr();
}

Style::AlignContent Style::ComputedValues::align_content() const
{
	if (auto p = element->GetLocalProperty(PropertyId::AlignContent))
		return static_cast<AlignContent>(p->Get<int>());
	return AlignContent::Stretch;
}

Style::AlignItems Style::ComputedValues::align_items() const
{
	if (auto p = element->GetLocalProperty(PropertyId::AlignItems))
		return static_cast<AlignItems>(p->Get<int>());
	return AlignItems::Stretch;
}

Style::AlignSelf Style::ComputedValues::align_self() const
{
	if (auto p = element->GetLocalProperty(PropertyId::AlignSelf))
		return static_cast<AlignSelf>(p->Get<int>());
	return AlignSelf::Auto;
}

Style::FlexDirection Style::ComputedValues::flex_direction() const
{
	if (auto p = element->GetLocalProperty(PropertyId::FlexDirection))
		return static_cast<FlexDirection>(p->Get<int>());
	return FlexDirection::Row;
}

Style::FlexWrap Style::ComputedValues::flex_wrap() const
{
	if (auto p = element->GetLocalProperty(PropertyId::FlexWrap))
		return static_cast<FlexWrap>(p->Get<int>());
	return FlexWrap::Nowrap;
}

Style::JustifyContent Style::ComputedValues::justify_content() const
{
	if (auto p = element->GetLocalProperty(PropertyId::JustifyContent))
		return static_cast<JustifyContent>(p->Get<int>());
	return JustifyContent::FlexStart;
}

float Style::ComputedValues::flex_grow() const
{
	if (auto p = element->GetLocalProperty(PropertyId::FlexGrow))
		return p->Get<float>();
	return 0.f;
}

float Style::ComputedValues::flex_shrink() const
{
	if (auto p = element->GetLocalProperty(PropertyId::FlexShrink))
		return p->Get<float>();
	return 1.f;
}

String Style::ComputedValues::text_overflow_string() const
{
	if (auto p = element->GetLocalProperty(PropertyId::TextOverflow))
		return p->Get<String>();
	return String();
}

} // namespace ui
