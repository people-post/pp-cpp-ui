#pragma once

#include <ui/Core/FontGlyph.h>
#include <ui/Core/StyleTypes.h>
#include <ui/Core/Types.h>

namespace ui {

using FontFaceHandleFreetype = uintptr_t;

struct FaceVariation {
	Style::FontWeight weight;
	uint16_t width;
	int named_instance_index;
};

inline bool operator<(const FaceVariation& a, const FaceVariation& b)
{
	if (a.weight == b.weight)
		return a.width < b.width;
	return a.weight < b.weight;
}

} // namespace ui
