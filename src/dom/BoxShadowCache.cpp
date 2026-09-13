#include "BoxShadowCache.h"
#include <ui/dom/Element.h>
#include <ui/style/ComputedValues.h>
#include <ui/paint/MeshUtilities.h>
#include <ui/base/Profiling.h>
#include <ui/paint/RenderManager.h>
#include "base/ControlledLifetimeResource.h"
#include "BoxShadowHash.h"
#include "paint/GeometryBoxShadow.h"

namespace ui {

struct BoxShadowCacheData {
	StableUnorderedMap<BoxShadowGeometryInfo, WeakPtr<BoxShadowRenderable>> handles;
};

static void ReleaseHandle(BoxShadowRenderable* handle);

BoxShadowRenderable::BoxShadowRenderable(const BoxShadowGeometryInfo& geometry_info) : cache_key(geometry_info) {}

BoxShadowRenderable::~BoxShadowRenderable()
{
	ReleaseHandle(this);
}

static ControlledLifetimeResource<BoxShadowCacheData> shadow_cache_data;

void BoxShadowCache::Initialize()
{
	shadow_cache_data.Initialize();
}

void BoxShadowCache::Shutdown()
{
	shadow_cache_data.Shutdown();
}

static BoxShadowGeometryInfo ResolveBoxShadowGeometry(Element* element, const CornerSizes& border_radius, ColourbPremultiplied background_color,
	const Array<ColourbPremultiplied, 4>& border_colors, float opacity)
{
	UI_ZoneScoped;

	// Find the box-shadow texture dimension and offset required to cover all box-shadows and element boxes combined.
	Vector2f element_offset_in_texture;
	Vector2i texture_dimensions;

	const Property* p_box_shadow = element->GetLocalProperty(PropertyId::BoxShadow);
	UI_ASSERT(p_box_shadow->value.GetType() == Variant::BOXSHADOWLIST);
	BoxShadowList shadow_list = p_box_shadow->value.Get<BoxShadowList>();

	// Resolve all lengths to px units.
	for (BoxShadow& shadow : shadow_list)
	{
		shadow.blur_radius = NumericValue(element->ResolveLength(shadow.blur_radius), Unit::PX);
		shadow.spread_distance = NumericValue(element->ResolveLength(shadow.spread_distance), Unit::PX);
		shadow.offset_x = NumericValue(element->ResolveLength(shadow.offset_x), Unit::PX);
		shadow.offset_y = NumericValue(element->ResolveLength(shadow.offset_y), Unit::PX);
	}

	{
		Vector2f extend_min;
		Vector2f extend_max;

		// Extend the render-texture to encompass box-shadow blur and spread.
		for (const BoxShadow& shadow : shadow_list)
		{
			if (!shadow.inset)
			{
				const float extend = 1.5f * shadow.blur_radius.number + shadow.spread_distance.number;
				const Vector2f offset = {shadow.offset_x.number, shadow.offset_y.number};
				extend_min = Math::Min(extend_min, offset - Vector2f(extend));
				extend_max = Math::Max(extend_max, offset + Vector2f(extend));
			}
		}

		Rectanglef texture_region;

		// Extend the render-texture further to cover all the element's boxes.
		for (int i = 0; i < element->BoxModel().GetNumBoxes(); i++)
		{
			const RenderBox box = element->GetRenderBox(BoxArea::Border, i);
			texture_region = texture_region.Join(Rectanglef::FromPositionSize(box.GetBorderOffset(), box.GetFillSize()));
		}

		texture_region = texture_region.Extend(-extend_min, extend_max);
		Math::ExpandToPixelGrid(texture_region);

		element_offset_in_texture = -texture_region.TopLeft();
		texture_dimensions = Vector2i(texture_region.Size());
	}

	// Since we can reuse textures across multiple box shadows with the same properties,
	// we need to copy the element's box shadow list and the background and border geometry.
	RenderBoxList padding_render_boxes{};
	RenderBoxList border_render_boxes{};

	for (int i = 0; i < element->BoxModel().GetNumBoxes(); i++)
	{
		padding_render_boxes.push_back(element->GetRenderBox(BoxArea::Padding, i));
		border_render_boxes.push_back(element->GetRenderBox(BoxArea::Border, i));
	}

	// Finally, create cache information
	BoxShadowGeometryInfo geometry_info;
	geometry_info.background_color = background_color;
	geometry_info.border_colors = border_colors;
	geometry_info.border_radius = border_radius;
	geometry_info.texture_dimensions = texture_dimensions;
	geometry_info.element_offset_in_texture = element_offset_in_texture;
	geometry_info.padding_render_boxes = std::move(padding_render_boxes);
	geometry_info.border_render_boxes = std::move(border_render_boxes);
	geometry_info.shadow_list = std::move(shadow_list);
	geometry_info.opacity = opacity;
	return geometry_info;
}


static SharedPtr<BoxShadowRenderable> GetOrCreateBoxShadow(RenderManager& render_manager, const BoxShadowGeometryInfo& info)
{
	UI_ZoneScoped;
	auto it_handle = shadow_cache_data->handles.find(info);
	if (it_handle != shadow_cache_data->handles.end())
	{
		SharedPtr<BoxShadowRenderable> result = it_handle->second.lock();
		UI_ASSERTMSG(result, "Failed to lock handle in Box Shadow cache");
		return result;
	}

	const auto iterator_inserted = shadow_cache_data->handles.emplace(info, WeakPtr<BoxShadowRenderable>());
	UI_ASSERTMSG(iterator_inserted.second, "Could not insert entry into the Box Shadow cache handle map, duplicate key.");
	const BoxShadowGeometryInfo& inserted_key = iterator_inserted.first->first;
	WeakPtr<BoxShadowRenderable>& inserted_weak_data_pointer = iterator_inserted.first->second;

	auto shadow_handle = MakeShared<BoxShadowRenderable>(inserted_key);
	GeometryBoxShadow::GenerateTexture(shadow_handle->texture, shadow_handle->background_border_geometry, render_manager, inserted_key);

	Mesh mesh;
	const byte alpha = byte(info.opacity * 255.f);
	MeshUtilities::GenerateQuad(mesh, -info.element_offset_in_texture, Vector2f(info.texture_dimensions), ColourbPremultiplied(alpha, alpha));
	shadow_handle->geometry = render_manager.MakeGeometry(std::move(mesh));

	inserted_weak_data_pointer = shadow_handle;
	return shadow_handle;
}

static void ReleaseHandle(BoxShadowRenderable* handle)
{
	// There are no longer any users of the cache entry uniquely identified by the handle address. Start from the
	// tip (i.e. per-color data) and remove that entry from its parent. Move up the cache ancestry and erase any
	// entries that no longer have any children.
	auto& handles = shadow_cache_data->handles;
	const BoxShadowGeometryInfo& key = handle->cache_key;

	auto it_handle = handles.find(key);
	UI_ASSERT(it_handle != handles.cend());

	handles.erase(it_handle);
}

SharedPtr<BoxShadowRenderable> BoxShadowCache::GetHandle(Element* element, const ComputedValues& computed)
{
	RenderManager* render_manager = element->GetRenderManager();
	if (!render_manager)
		return {};

	ColourbPremultiplied background_color = computed.background_color().ToPremultiplied();
	Array<ColourbPremultiplied, 4> border_colors = {
		computed.border_top_color().ToPremultiplied(),
		computed.border_right_color().ToPremultiplied(),
		computed.border_bottom_color().ToPremultiplied(),
		computed.border_left_color().ToPremultiplied(),
	};
	const CornerSizes border_radius = computed.border_radius();
	BoxShadowGeometryInfo geom_info = ResolveBoxShadowGeometry(element, border_radius, background_color, border_colors, computed.opacity());
	return GetOrCreateBoxShadow(*render_manager, geom_info);
}

} // namespace ui
