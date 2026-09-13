#pragma once

#include <ui/Debugger/Header.h>

namespace ui {

class Context;

namespace Debugger {

	/// Initialises the debug plugin. The debugger will be loaded into the given context.
	/// @param[in] host_context pp-cpp-ui context to load the debugger into. The debugging tools will be displayed on this context. If this context is
	///     destroyed, the debugger will be released.
	/// @return True if the debugger was successfully initialised
	UI_DEBUGGER_API bool Initialise(Context* host_context);

	/// Shuts down the debugger.
	/// @note The debugger is automatically shutdown during the call to ui::Shutdown(), calling this is only necessary to shutdown the debugger early
	///     or to re-initialize the debugger on another host context.
	UI_DEBUGGER_API void Shutdown();

	/// Sets the context to be debugged.
	/// @param[in] context The context to be debugged.
	/// @return True if the debugger is initialised and the context was switched, false otherwise.
	UI_DEBUGGER_API bool SetContext(Context* context);

	/// Sets the visibility of the debugger.
	/// @param[in] visibility True to show the debugger, false to hide it.
	UI_DEBUGGER_API void SetVisible(bool visibility);
	/// Returns the visibility of the debugger.
	/// @return True if the debugger is visible, false if not.
	UI_DEBUGGER_API bool IsVisible();

} // namespace Debugger
} // namespace ui
