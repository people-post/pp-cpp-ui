#pragma once

#include <ui/font/FontGlyph.h>
#include <ui/style/StyleTypes.h>
#include <ui/base/Types.h>
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
