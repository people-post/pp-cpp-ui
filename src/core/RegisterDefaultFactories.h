#pragma once

namespace ui {

/// Registers default widget / data / XML control factories owned by the composition root.
/// Call after Factory::Initialise(); call Shutdown before Factory::Shutdown().
namespace RegisterDefaultFactories {

void Initialise();
void Shutdown();

} // namespace RegisterDefaultFactories

} // namespace ui
