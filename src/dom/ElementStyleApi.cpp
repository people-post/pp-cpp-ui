// Element — style façade (definitions only).
#include <ui/dom/Element.h>
#include <ui/dom/ElementDocument.h>
#include <ui/dom/ElementScroll.h>
#include <ui/style/StyleSheet.h>
#include <ui/style/StyleSheetSpecification.h>
#include <ui/style/ComputedValues.h>
#include <ui/style/PropertiesIteratorView.h>
#include "ElementMeta.h"
#include "ElementStyle.h"
#include "style/PropertiesIterator.h"

namespace ui {

void Element::SetClass(const String& class_name, bool activate)
{
	if (Style().SetClass(class_name, activate))
		DirtyDefinition(DirtyNodes::SelfAndSiblings);
}
bool Element::IsClassSet(const String& class_name) const
{
	return Style().IsClassSet(class_name);
}
void Element::SetClassNames(const String& class_names)
{
	SetAttribute("class", class_names);
}
String Element::GetClassNames() const
{
	return Style().GetClassNames();
}
const StyleSheet* Element::GetStyleSheet() const
{
	if (ElementDocument* document = GetOwnerDocument())
		return document->GetStyleSheet();
	return nullptr;
}
bool Element::SetProperty(const String& name, const String& value)
{
	// The name may be a shorthand giving us multiple underlying properties
	PropertyDictionary properties;
	if (!StyleSheetSpecification::ParsePropertyDeclaration(properties, name, value))
	{
		Log::Message(Log::LT_WARNING, "Syntax error parsing inline property declaration '%s: %s;'.", name.c_str(), value.c_str());
		return false;
	}
	for (auto& property : properties.GetProperties())
	{
		if (!Style().SetProperty(property.first, property.second))
			return false;
	}
	return true;
}
bool Element::SetProperty(PropertyId id, const Property& property)
{
	return Style().SetProperty(id, property);
}
void Element::RemoveProperty(const String& name)
{
	auto property_id = StyleSheetSpecification::GetPropertyId(name);
	if (property_id != PropertyId::Invalid)
		Style().RemoveProperty(property_id);
	else
	{
		auto shorthand_id = StyleSheetSpecification::GetShorthandId(name);
		if (shorthand_id != ShorthandId::Invalid)
		{
			auto property_id_set = StyleSheetSpecification::GetShorthandUnderlyingProperties(shorthand_id);
			for (auto it = property_id_set.begin(); it != property_id_set.end(); ++it)
				Style().RemoveProperty(*it);
		}
	}
}
void Element::RemoveProperty(PropertyId id)
{
	Style().RemoveProperty(id);
}
const Property* Element::GetProperty(const String& name)
{
	return Style().GetProperty(StyleSheetSpecification::GetPropertyId(name));
}
const Property* Element::GetProperty(PropertyId id)
{
	return Style().GetProperty(id);
}
const Property* Element::GetLocalProperty(const String& name)
{
	return Style().GetLocalProperty(StyleSheetSpecification::GetPropertyId(name));
}
const Property* Element::GetLocalProperty(PropertyId id)
{
	return Style().GetLocalProperty(id);
}
const PropertyMap& Element::GetLocalStyleProperties()
{
	return Style().GetLocalStyleProperties();
}
float Element::ResolveLength(NumericValue value)
{
	float result = 0.f;
	if (Any(value.unit & Unit::LENGTH))
		result = Style().ResolveNumericValue(value, 0.f);
	return result;
}
float Element::ResolveNumericValue(NumericValue value, float base_value)
{
	float result = 0.f;
	if (Any(value.unit & Unit::NUMERIC))
		result = Style().ResolveNumericValue(value, base_value);
	return result;
}
Vector2f Element::GetContainingBlock()
{
	Vector2f containing_block;

	if (offset_parent)
	{
		using namespace Style;
		Position position_property = GetPosition();
		const Box& parent_box = offset_parent->GetBox();

		if (position_property == Position::Static || position_property == Position::Relative || position_property == Position::Sticky)
		{
			containing_block = parent_box.GetSize();
			containing_block.x -= Scroll().GetScrollbarSize(ElementScroll::VERTICAL);
			containing_block.y -= Scroll().GetScrollbarSize(ElementScroll::HORIZONTAL);
		}
		else if (position_property == Position::Absolute || position_property == Position::Fixed)
		{
			containing_block = parent_box.GetSize(BoxArea::Padding);
		}
	}
	else if (Context* context = GetContext())
	{
		containing_block = Vector2f(context->GetDimensions());
	}

	return containing_block;
}
Style::Position Element::GetPosition()
{
	return meta->computed_values.position();
}
Style::Float Element::GetFloat()
{
	return meta->computed_values.float_();
}
Style::Display Element::GetDisplay()
{
	return meta->computed_values.display();
}
float Element::GetLineHeight()
{
	return meta->computed_values.line_height().value;
}
void Element::SetPseudoClass(const String& pseudo_class, bool activate)
{
	if (Style().SetPseudoClass(pseudo_class, activate, false))
	{
		// Include siblings in case of RCSS presence of sibling combinators '+', '~'.
		DirtyDefinition(DirtyNodes::SelfAndSiblings);
		OnPseudoClassChange(pseudo_class, activate);
	}
}
bool Element::IsPseudoClassSet(const String& pseudo_class) const
{
	return Style().IsPseudoClassSet(pseudo_class);
}
bool Element::ArePseudoClassesSet(const StringList& pseudo_classes) const
{
	for (const String& pseudo_class : pseudo_classes)
	{
		if (!IsPseudoClassSet(pseudo_class))
			return false;
	}

	return true;
}
StringList Element::GetActivePseudoClasses() const
{
	const PseudoClassMap& pseudo_classes = Style().GetActivePseudoClasses();
	StringList names;
	names.reserve(pseudo_classes.size());
	for (auto& pseudo_class : pseudo_classes)
	{
		names.push_back(pseudo_class.first);
	}

	return names;
}
void Element::OverridePseudoClass(Element* element, const String& pseudo_class, bool activate)
{
	UI_ASSERT(element);
	element->Style().SetPseudoClass(pseudo_class, activate, true);
}
void Element::DirtyDefinition(DirtyNodes dirty_nodes)
{
	switch (dirty_nodes)
	{
	case DirtyNodes::Self: dirty_definition = true; break;
	case DirtyNodes::SelfAndSiblings:
		dirty_definition = true;
		if (parent)
			parent->dirty_child_definitions = true;
		break;
	}
}
void Element::UpdateDefinition()
{
	if (dirty_definition)
	{
		dirty_definition = false;

		// Dirty definition implies all our descendent elements. Anything that can change the definition of this element can also change the
		// definition of any descendants due to the presence of RCSS descendant or child combinators. In principle this also applies to sibling
		// combinators, but those are handled during the DirtyDefinition call.
		dirty_child_definitions = true;

		Style().UpdateDefinition();
	}

	if (dirty_child_definitions)
	{
		dirty_child_definitions = false;
		for (const ElementPtr& child : children)
			child->dirty_definition = true;
	}
}
void Element::OnStyleSheetChangeRecursive()
{
	Effects().DirtyEffects();

	OnStyleSheetChange();

	// Now dirty all of our descendants.
	const int num_children = GetNumChildren(true);
	for (int i = 0; i < num_children; ++i)
		GetChild(i)->OnStyleSheetChangeRecursive();
}
void Element::OnDpRatioChangeRecursive()
{
	Effects().DirtyEffects();
	Style().DirtyPropertiesWithUnits(Unit::DP_SCALABLE_LENGTH);

	OnDpRatioChange();

	// Now dirty all of our descendants.
	const int num_children = GetNumChildren(true);
	for (int i = 0; i < num_children; ++i)
		GetChild(i)->OnDpRatioChangeRecursive();
}
void Element::DirtyFontFaceRecursive()
{
	// Dirty the font size to force the element to update the face handle during the next Update(), and update any existing text geometry.
	Style().DirtyProperty(PropertyId::FontSize);
	meta->computed_values.font_face_handle(0);

	const int num_children = GetNumChildren(true);
	for (int i = 0; i < num_children; ++i)
		GetChild(i)->DirtyFontFaceRecursive();
}
PropertiesIteratorView Element::IterateLocalProperties() const
{
	return PropertiesIteratorView(MakeUnique<PropertiesIterator>(Style().Iterate()));
}

} // namespace ui
