#include "TestConfig.h"
#include <ui/base/StringUtilities.h>
#include <ui/base/Types.h>
#include <PlatformExtensions.h>
#include <Shell.h>
#include <cstdlib>

ui::String GetCompareInputDirectory()
{
	ui::String input_directory;

	if (const char* env_variable = std::getenv("UI_VISUAL_TESTS_COMPARE_DIRECTORY"))
		input_directory = env_variable;
	else
		input_directory = PlatformExtensions::FindSamplesRoot() + "../Tests/Output";

	return input_directory;
}

ui::String GetCaptureOutputDirectory()
{
	ui::String output_directory;

	if (const char* env_variable = std::getenv("UI_VISUAL_TESTS_CAPTURE_DIRECTORY"))
		output_directory = env_variable;
	else
		output_directory = PlatformExtensions::FindSamplesRoot() + "../Tests/Output";

	return output_directory;
}

ui::StringList GetTestInputDirectories()
{
	const ui::String samples_root = PlatformExtensions::FindSamplesRoot();

	ui::StringList directories = {samples_root + "../Tests/Data/VisualTests"};

	if (const char* env_variable = std::getenv("UI_VISUAL_TESTS_RML_DIRECTORIES"))
		ui::StringUtilities::ExpandString(directories, env_variable, ',');

	return directories;
}
