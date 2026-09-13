#include <ui/base/DataExpressionTools.h>

namespace ui {

const char* DataExpressionTools::ParseDataBrackets(bool& inside_brackets, bool& inside_string, char c, char previous)
{
	if (inside_brackets)
	{
		if (c == '\'')
			inside_string = !inside_string;

		if (!inside_string)
		{
			if (c == '}' && previous == '}')
				inside_brackets = false;

			else if (c == '{' && previous == '{')
				return "Nested double curly brackets are illegal.";

			else if (previous == '}' && c != '}' && c != '\'')
				return "Single closing curly bracket encountered, use double curly brackets to close an expression.";

			else if (previous == '/' && c == '>')
				return "Closing double curly brackets not found, XML end node encountered first.";

			else if (previous == '<' && c == '/')
				return "Closing double curly brackets not found, XML end node encountered first.";
		}
	}
	else
	{
		if (c == '{' && previous == '{')
		{
			inside_brackets = true;
		}
		else if (c == '}' && previous == '}')
		{
			return "Closing double curly brackets encountered outside an expression.";
		}
	}

	return nullptr;
}

} // namespace ui
