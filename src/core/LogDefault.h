#pragma once

#include <ui/Core/Log.h>
#include <ui/Core/Types.h>

namespace Rml {

/**
    Provides a platform-dependent default implementation for message logging.
 */

class LogDefault {
public:
	static bool LogMessage(Log::Type type, const String& message);
};

} // namespace Rml
