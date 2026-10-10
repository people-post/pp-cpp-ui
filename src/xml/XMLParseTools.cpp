#include "XMLParseTools.h"
#include <ui/base/DataExpressionTools.h>
#include <ui/dom/ElementDocument.h>
#include <ui/base/StreamMemory.h>
#include <ui/base/StringUtilities.h>
#include <ui/base/Types.h>
#include "dom/Template.h"
#include "dom/TemplateCache.h"
#include <ctype.h>
#include <string.h>

namespace ui {

const char* XMLParseTools::FindTag(const char* tag, const char* string, bool closing_tag)
{
	const size_t length = strlen(tag);
	const char* ptr = string;
	bool found_closing = false;

	while (*ptr)
	{
		// Check if the first character matches
		if (tolower((*ptr)) == tag[0])
		{
			// If it does, check the whole word
			if (StringUtilities::StringCompareCaseInsensitive(StringView(ptr, ptr + length), StringView(tag, tag + length)))
			{
				// Check for opening <, loop back in the string skipping white space and forward slashes if
				// we're looking for the closing tag
				const char* tag_start = ptr - 1;
				while (tag_start > string && (StringUtilities::IsWhitespace(*tag_start) || *tag_start == '/'))
				{
					if (*tag_start == '/')
						found_closing = true;
					tag_start--;
				}

				// If the character we're looking at is a <, and found closing matches closing tag,
				// its the tag we're looking for
				if (*tag_start == '<' && found_closing == closing_tag)
					return tag_start;

				// Otherwise, keep looking
			}
		}
		ptr++;
	}

	return nullptr;
}

bool XMLParseTools::ReadAttribute(const char*& string, String& name, String& value)
{
	const char* ptr = string;

	name = "";
	value = "";

	// Skip whitespace
	while (StringUtilities::IsWhitespace(*ptr))
		ptr++;

	// Look for the end of the attribute name
	bool found_whitespace = false;
	while (*ptr != '=' && *ptr != '>' && (!found_whitespace || StringUtilities::IsWhitespace(*ptr)))
	{
		if (StringUtilities::IsWhitespace(*ptr))
			found_whitespace = true;
		else
			name += *ptr;
		ptr++;
	}
	if (*ptr == '>')
		return false;

	// If we stopped on an equals, parse the value
	if (*ptr == '=')
	{
		// Skip over white space, ='s and quotes
		bool quoted = false;
		while (StringUtilities::IsWhitespace(*ptr) || *ptr == '\'' || *ptr == '"' || *ptr == '=')
		{
			if (*ptr == '\'' || *ptr == '"')
				quoted = true;
			ptr++;
		}
		if (*ptr == '>')
			return false;

		// Store the value
		while (*ptr != '\'' && *ptr != '"' && *ptr != '>' && (*ptr != ' ' || quoted))
		{
			value += *ptr++;
		}
		if (*ptr == '>')
			return false;

		// Advance passed the quote
		if (quoted)
			ptr++;
	}
	else
	{
		ptr--;
	}

	// Update the string pointer
	string = ptr;

	return true;
}

// Limits on template expansion, counted per outermost XML parser (a document load or an inner RML parse).
static constexpr int MAX_TEMPLATE_EXPANSION_DEPTH = 16;
static constexpr int MAX_TEMPLATE_EXPANSION_COUNT = 256;

struct TemplateExpansionState {
	int active_parsers = 0;
	int depth = 0;
	int count = 0;
	bool depth_reported = false;
	bool count_reported = false;
};
static thread_local TemplateExpansionState template_expansion;

void XMLParseTools::BeginParser()
{
	if (template_expansion.active_parsers++ == 0)
		template_expansion = TemplateExpansionState{1};
}

void XMLParseTools::EndParser()
{
	template_expansion.active_parsers--;
}

Element* XMLParseTools::ParseTemplate(Element* element, const String& template_name)
{
	// Load the template, and parse it
	Template* parse_template = TemplateCache::GetTemplate(template_name);
	if (!parse_template)
	{
		Log::ParseError(element->GetOwnerDocument()->GetSourceURL(), -1, "Failed to find template '%s'.", template_name.c_str());
		return element;
	}

	TemplateExpansionState& state = template_expansion;
	if (state.depth >= MAX_TEMPLATE_EXPANSION_DEPTH)
	{
		if (!state.depth_reported)
			Log::ParseError(element->GetOwnerDocument()->GetSourceURL(), -1, "Template '%s' exceeds the maximum template expansion depth (%d).",
				template_name.c_str(), MAX_TEMPLATE_EXPANSION_DEPTH);
		state.depth_reported = true;
		return element;
	}
	if (state.count >= MAX_TEMPLATE_EXPANSION_COUNT)
	{
		if (!state.count_reported)
			Log::ParseError(element->GetOwnerDocument()->GetSourceURL(), -1, "Template '%s' exceeds the maximum number of template expansions (%d).",
				template_name.c_str(), MAX_TEMPLATE_EXPANSION_COUNT);
		state.count_reported = true;
		return element;
	}

	state.count++;
	state.depth++;
	Element* result = parse_template->ParseTemplate(element);
	state.depth--;

	return result;
}

const char* XMLParseTools::ParseDataBrackets(bool& inside_brackets, bool& inside_string, char c, char previous)
{
	return DataExpressionTools::ParseDataBrackets(inside_brackets, inside_string, c, previous);
}


} // namespace ui
