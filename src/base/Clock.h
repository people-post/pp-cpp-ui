#pragma once

#include <ui/Core/Header.h>

namespace ui {

/**
    pp-cpp-ui's Interface to Time.
 */
class Clock {
public:
	/// Get the elapsed time since application startup
	/// @return Seconds elapsed since application startup.
	UI_CORE_API static double GetElapsedTime();
};

} // namespace ui
