#include <ui/base/TextInputHandler.h>

namespace ui {

static TextInputHandler* g_text_input_handler = nullptr;

void SetTextInputHandler(TextInputHandler* text_input_handler)
{
	g_text_input_handler = text_input_handler;
}

TextInputHandler* GetTextInputHandler()
{
	return g_text_input_handler;
}

} // namespace ui
