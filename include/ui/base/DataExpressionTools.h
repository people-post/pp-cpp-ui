#pragma once

#include <ui/base/Header.h>

namespace ui {

/// Helpers for scanning data-expression brackets (`{{ ... }}`) in markup/text.
namespace DataExpressionTools {

/// Advance bracket/string state for one character.
/// Returns nullptr on success, or a static error string on failure.
UI_CORE_API const char* ParseDataBrackets(bool& inside_brackets, bool& inside_string, char c, char previous);

} // namespace DataExpressionTools

} // namespace ui
