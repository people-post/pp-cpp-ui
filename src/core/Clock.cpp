#include "Clock.h"
#include <ui/Core/Core.h>
#include <ui/Core/SystemInterface.h>

namespace ui {

UI_CORE_API double Clock::GetElapsedTime()
{
	SystemInterface* system_interface = GetSystemInterface();
	if (system_interface != nullptr)
		return system_interface->GetElapsedTime();
	else
		return 0;
}

} // namespace ui
