#pragma once

#include <ui/Core.h>

using FontGlyphIndex = uint32_t;

struct FontGlyphData
{
	ui::FontGlyph bitmap;
	ui::Character character;
};

struct FontGlyphReference
{
	const ui::FontGlyph* bitmap;
	ui::Character character;
};

struct FontClusterGlyphData
{
	FontGlyphIndex glyph_index;
	FontGlyphData glyph_data;
};

using FontGlyphMap = ui::UnorderedMap<FontGlyphIndex, FontGlyphData>;
using FallbackFontGlyphMap = ui::UnorderedMap<ui::Character, ui::FontGlyph>;
using FallbackFontClusterGlyphsMap = ui::UnorderedMap<ui::String, ui::Vector<FontClusterGlyphData>>;
using FallbackFontClusterGlyphLookupMap = ui::UnorderedMap<uint64_t, const ui::FontGlyph*>;

struct FontGlyphMaps {
	const FontGlyphMap* glyphs;
	const FallbackFontGlyphMap* fallback_glyphs;
	const FallbackFontClusterGlyphLookupMap* fallback_cluster_glyphs;
};

inline uint64_t GetFallbackFontClusterGlyphLookupID(FontGlyphIndex glyph_index, ui::Character character)
{
	// Combine 32-bit glyph index and 32-bit character into a single 64-bit integer.
	return (static_cast<uint64_t>(glyph_index) << (sizeof(ui::Character) * 8)) | static_cast<uint64_t>(character);
}
