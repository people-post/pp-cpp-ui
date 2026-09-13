#include "Clock.h"
#include <ui/core/Core.h>
#include <ui/core/SystemInterface.h>
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
