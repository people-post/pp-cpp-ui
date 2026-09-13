#pragma once

#include <ui/Core/Context.h>
#include <ui/Core/Input.h>
#include <ui/Core/Types.h>

/**
    Provides common functionality required for the built-in pp-cpp-ui samples.
 */
namespace Shell {

// Initializes and sets a custom file interface used for locating the included pp-cpp-ui asset files.
bool Initialize();
// Destroys all resources constructed by the shell.
void Shutdown();

// Loads the fonts included with the pp-cpp-ui samples.
void LoadFonts();

// Process key down events to handle shortcuts common to all samples.
// @return True if the event is still propagating, false if it was handled here.
bool ProcessKeyDownShortcuts(ui::Context* context, ui::Input::KeyIdentifier key, int key_modifier, float native_dp_ratio, bool priority);

} // namespace Shell
