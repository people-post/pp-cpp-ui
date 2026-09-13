#pragma once

#include <ui/base/Types.h>
namespace RendererExtensions {

// Extensions used by the test suite
struct Image {
	int width = 0;
	int height = 0;
	int num_components = 0;
	ui::UniquePtr<ui::byte[]> data;
};
Image CaptureScreen();

} // namespace RendererExtensions
