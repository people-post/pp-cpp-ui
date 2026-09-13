#pragma once

#include <ui/style/ComputedValues.h>
#include <ui/base/NumericValue.h>
#include <ui/paint/RenderBox.h>
#include <ui/base/Types.h>
#include <ui/style/Unit.h>
#include <ui/base/Utilities.h>
#include "BoxShadowCache.h"
#include "paint/GeometryBoxShadow.h"

namespace std {

template <>
struct hash<::ui::Unit> {
	using utype = underlying_type_t<::ui::Unit>;
	size_t operator()(const ::ui::Unit& t) const noexcept
	{
		hash<utype> h;
		return h(static_cast<utype>(t));
	}
};

template <>
struct hash<::ui::Vector2i> {
	size_t operator()(const ::ui::Vector2i& v) const noexcept
	{
		using namespace ::ui::Utilities;
		size_t seed = hash<int>{}(v.x);
		HashCombine(seed, v.y);
		return seed;
	}
};
template <>
struct hash<::ui::Vector2f> {
	size_t operator()(const ::ui::Vector2f& v) const noexcept
	{
		using namespace ::ui::Utilities;
		size_t seed = hash<float>{}(v.x);
		HashCombine(seed, v.y);
		return seed;
	}
};
template <>
struct hash<::ui::Colourb> {
	size_t operator()(const ::ui::Colourb& v) const noexcept { return static_cast<size_t>(hash<uint32_t>{}(reinterpret_cast<const uint32_t&>(v))); }
};
template <>
struct hash<::ui::ColourbPremultiplied> {
	size_t operator()(const ::ui::ColourbPremultiplied& v) const noexcept
	{
		return static_cast<size_t>(hash<uint32_t>{}(reinterpret_cast<const uint32_t&>(v)));
	}
};

template <>
struct hash<::ui::NumericValue> {
	size_t operator()(const ::ui::NumericValue& v) const noexcept
	{
		using namespace ::ui::Utilities;
		size_t seed = hash<float>{}(v.number);
		HashCombine(seed, v.unit);
		return seed;
	}
};

template <>
struct hash<::ui::BoxShadow> {
	size_t operator()(const ::ui::BoxShadow& s) const noexcept
	{
		using namespace ::ui;
		using namespace ::ui::Utilities;
		size_t seed = std::hash<ColourbPremultiplied>{}(s.color);

		HashCombine(seed, s.offset_x);
		HashCombine(seed, s.offset_y);
		HashCombine(seed, s.blur_radius);
		HashCombine(seed, s.spread_distance);
		HashCombine(seed, s.inset);
		return seed;
	}
};

template <>
struct hash<::ui::RenderBox> {
	size_t operator()(const ::ui::RenderBox& box) const noexcept
	{
		using namespace ::ui::Utilities;
		static auto HashArray4 = [](const ::ui::Array<float, 4>& arr) -> size_t {
			size_t seed = 0;
			for (const auto& v : arr)
				HashCombine(seed, v);
			return seed;
		};

		size_t seed = 0;
		HashCombine(seed, box.GetFillSize());
		HashCombine(seed, box.GetBorderOffset());
		HashCombine(seed, HashArray4(box.GetBorderRadius()));
		HashCombine(seed, HashArray4(box.GetBorderWidths()));
		return seed;
	}
};

template <>
struct hash<::ui::BoxShadowGeometryInfo> {
	size_t operator()(const ::ui::BoxShadowGeometryInfo& in) const noexcept
	{
		using namespace ::ui::Utilities;
		size_t seed = size_t(849128392);

		HashCombine(seed, in.background_color);
		for (const auto& v : in.border_colors)
		{
			HashCombine(seed, v);
		}

		for (const auto& v : in.border_radius)
		{
			HashCombine(seed, v);
		}

		HashCombine(seed, in.texture_dimensions);
		HashCombine(seed, in.element_offset_in_texture);

		for (const auto& v : in.padding_render_boxes)
		{
			HashCombine(seed, v);
		}
		for (const auto& v : in.border_render_boxes)
		{
			HashCombine(seed, v);
		}
		for (const ::ui::BoxShadow& v : in.shadow_list)
		{
			HashCombine(seed, v);
		}
		HashCombine(seed, in.opacity);
		return seed;
	}
};

} // namespace std
