#pragma once

#include <ui/Core/Types.h>

namespace PlatformExtensions {

Rml::String FindSamplesRoot();

Rml::StringList ListDirectories(const Rml::String& in_directory);
Rml::StringList ListFiles(const Rml::String& in_directory, const Rml::String& extension = Rml::String());

} // namespace PlatformExtensions
