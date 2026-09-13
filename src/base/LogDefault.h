#pragma once

#include <ui/base/Log.h>
#include <ui/base/Types.h>
namespace ui {

/**
    Provides a platform-dependent default implementation for message logging.
 */

class LogDefault {
public:
	static bool LogMessage(Log::Type type, const String& message);
};

} // namespace ui
