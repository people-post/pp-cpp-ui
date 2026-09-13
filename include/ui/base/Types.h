#pragma once

#include <ui/Config/Config.h>
#include <ui/base/Traits.h>
#include <cstdlib>
#include <memory>
#include <stddef.h>
#include <stdint.h>

namespace ui {

// Commonly used basic types
using byte = unsigned char;
using ScriptObject = void*;

enum class Character : char32_t { Null, Replacement = 0xfffd }; // Unicode code point
enum class BoxArea { Margin, Border, Padding, Content, Auto };

} // namespace ui

#include <ui/base/Colour.h>
#include <ui/base/Matrix4.h>
#include <ui/base/ObserverPtr.h>
#include <ui/base/Rectangle.h>
#include <ui/base/Span.h>
#include <ui/base/Vector2.h>
#include <ui/base/Vector3.h>
#include <ui/base/Vector4.h>

namespace ui {

// Color and linear algebra
enum class ColorFormat { RGBA8, A8 };
using Colourf = Colour<float, 1, false>;
using Colourb = Colour<byte, 255, false>;
using ColourbPremultiplied = Colour<byte, 255, true>;
using Vector2i = Vector2<int>;
using Vector2f = Vector2<float>;
using Vector3i = Vector3<int>;
using Vector3f = Vector3<float>;
using Vector4i = Vector4<int>;
using Vector4f = Vector4<float>;
using Rectanglei = Rectangle<int>;
using Rectanglef = Rectangle<float>;
using ColumnMajorMatrix4f = Matrix4<float, ColumnMajorStorage<float>>;
using RowMajorMatrix4f = Matrix4<float, RowMajorStorage<float>>;
using Matrix4f = UI_MATRIX4_TYPE;

// Common classes
class Element;
class ElementInstancer;
class ElementAnimation;
class RenderManager;
class Texture;
class Context;
class Event;
class Property;
class Variant;
class Transform;
class PropertyIdSet;
class Decorator;
class FontEffect;
class StringView;
struct Animation;
struct Transition;
struct TransitionList;
struct DecoratorDeclarationList;
struct FilterDeclarationList;
struct ColorStop;
struct BoxShadow;
enum class EventId : uint16_t;
enum class PropertyId : uint8_t;
enum class MediaQueryId : uint8_t;
enum class FamilyId : int;
using TouchId = uintptr_t;

// Types for external interfaces.
using FileHandle = uintptr_t;
using TextureHandle = uintptr_t;
using CompiledGeometryHandle = uintptr_t;
using CompiledFilterHandle = uintptr_t;
using CompiledShaderHandle = uintptr_t;
using DecoratorDataHandle = uintptr_t;
using FontFaceHandle = uintptr_t;
using FontEffectsHandle = uintptr_t;
using LayerHandle = uintptr_t;

using ElementPtr = UniqueReleaserPtr<Element>;
using ContextPtr = UniqueReleaserPtr<Context>;
using EventPtr = UniqueReleaserPtr<Event>;

struct Touch {
	TouchId identifier;
	Vector2f position;
};
using TouchList = Vector<Touch>;

enum class StableVectorIndex : uint32_t { Invalid = uint32_t(-1) };
enum class TextureFileIndex : uint32_t { Invalid = uint32_t(-1) };

// Container types for common classes
using ElementList = Vector<Element*>;
using OwnedElementList = Vector<ElementPtr>;
using VariantList = Vector<Variant>;
using ElementAnimationList = Vector<ElementAnimation>;

using AttributeNameList = SmallUnorderedSet<String>;
using PropertyMap = UnorderedMap<PropertyId, Property>;

using Dictionary = SmallUnorderedMap<String, Variant>;
using ElementAttributes = Dictionary;
using XMLAttributes = Dictionary;

using AnimationList = Vector<Animation>;
using FontEffectList = Vector<SharedPtr<const FontEffect>>;
struct FontEffects {
	FontEffectList list;
	String value;
};
using ColorStopList = Vector<ColorStop>;
using BoxShadowList = Vector<BoxShadow>;
using FilterHandleList = Vector<CompiledFilterHandle>;

// Additional smart pointers
using TransformPtr = SharedPtr<Transform>;
using DecoratorsPtr = SharedPtr<const DecoratorDeclarationList>;
using FiltersPtr = SharedPtr<const FilterDeclarationList>;
using FontEffectsPtr = SharedPtr<const FontEffects>;

// Data binding types
class DataView;
using DataViewPtr = UniqueReleaserPtr<DataView>;
class DataController;
using DataControllerPtr = UniqueReleaserPtr<DataController>;

} // namespace ui

namespace std {
// Hash specialization for enum class types (required on some older compilers)
template <>
struct hash<::ui::PropertyId> {
	using utype = ::std::underlying_type_t<::ui::PropertyId>;
	size_t operator()(const ::ui::PropertyId& t) const noexcept
	{
		::std::hash<utype> h;
		return h(static_cast<utype>(t));
	}
};
template <>
struct hash<::ui::Character> {
	using utype = ::std::underlying_type_t<::ui::Character>;
	size_t operator()(const ::ui::Character& t) const noexcept
	{
		::std::hash<utype> h;
		return h(static_cast<utype>(t));
	}
};
template <>
struct hash<::ui::FamilyId> {
	using utype = ::std::underlying_type_t<::ui::FamilyId>;
	size_t operator()(const ::ui::FamilyId& t) const noexcept
	{
		::std::hash<utype> h;
		return h(static_cast<utype>(t));
	}
};
} // namespace std
