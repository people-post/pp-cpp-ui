#pragma once

#include "Header.h"
#include "Types.h"

namespace ui {

/**
    The element that makes up all geometry sent to the renderer.
 */

struct UI_CORE_API Vertex {
	/// Two-dimensional position of the vertex (usually in pixels).
	Vector2f position;
	/// RGBA-ordered 8-bit/channel colour with premultiplied alpha.
	ColourbPremultiplied colour;
	/// Texture coordinate for any associated texture.
	Vector2f tex_coord;

	friend bool operator==(const Vertex& lhs, const Vertex& rhs)
	{
		return lhs.position == rhs.position && lhs.colour == rhs.colour && lhs.tex_coord == rhs.tex_coord;
	}
	friend bool operator!=(const Vertex& lhs, const Vertex& rhs) { return !(lhs == rhs); }
};

} // namespace ui
