#pragma once

#include <ui/base/Types.h>

namespace ui {

class Element;

/**
    Layout-facing interface for text nodes that participate in inline layout.
    Implemented by ElementText (text module). Keeps layout free of concrete text types.
 */
class LayoutTextElement {
public:
	virtual ~LayoutTextElement() = default;

	virtual Element* GetLayoutElement() = 0;

	/// Generates a line of text for layout. See ElementText::GenerateLine.
	virtual bool GenerateLine(String& line, int& line_length, float& line_width, int line_begin, float maximum_line_width,
		float right_spacing_width, bool trim_whitespace_prefix, bool decode_escape_characters, bool allow_empty) = 0;

	virtual void ClearLines() = 0;
	virtual void AddLine(Vector2f line_position, String line) = 0;

	/// Short string for layout debug dumps.
	virtual String GetDebugText() const = 0;
};

} // namespace ui
