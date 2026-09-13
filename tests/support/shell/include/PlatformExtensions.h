#pragma once

#include <ui/Core/Types.h>

namespace PlatformExtensions {

ui::String FindSamplesRoot();

ui::StringList ListDirectories(const ui::String& in_directory);
ui::StringList ListFiles(const ui::String& in_directory, const ui::String& extension = ui::String());

} // namespace PlatformExtensions
