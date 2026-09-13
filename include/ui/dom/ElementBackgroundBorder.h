#pragma once

#include <ui/base/Header.h>
#include <ui/paint/CallbackTexture.h>
#include <ui/paint/Geometry.h>
#include <ui/base/Types.h>
namespace ui {

class Element;
struct BoxShadowRenderable;

/**
    Generates and renders an element's background and border geometry.

    Prefer `element->BackgroundBorder()`.
 */

class UI_CORE_API ElementBackgroundBorder {
public:
	ElementBackgroundBorder();
	~ElementBackgroundBorder() = default;

	ElementBackgroundBorder(const ElementBackgroundBorder&) = delete;
	ElementBackgroundBorder& operator=(const ElementBackgroundBorder&) = delete;
	void Render(Element* element);

	void DirtyBackground();
	void DirtyBorder();

	Geometry* GetClipGeometry(Element* element, BoxArea clip_area);

private:
	enum class BackgroundType { BackgroundBorder, BoxShadowAndBackgroundBorder, ClipBorder, ClipPadding, ClipContent, Count };
	struct Background {
		Geometry geometry;
		Texture texture;
		SharedPtr<BoxShadowRenderable> box_shadow_and_background_border;
	};

	Background* GetBackground(BackgroundType type);
	Background& GetOrCreateBackground(BackgroundType type);
	void EraseBackground(BackgroundType type);

	void GenerateGeometry(Element* element);

	bool background_dirty = false;
	bool border_dirty = false;

	StableMap<BackgroundType, Background> backgrounds;
};

} // namespace ui
