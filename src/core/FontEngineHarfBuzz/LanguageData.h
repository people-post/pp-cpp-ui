#pragma once

#include <ui/Core.h>

enum class TextFlowDirection {
	LeftToRight,
	RightToLeft,
};

struct LanguageData {
	ui::String script_code;
	TextFlowDirection text_flow_direction;
};

using LanguageDataMap = ui::UnorderedMap<ui::String, LanguageData>;
