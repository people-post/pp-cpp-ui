#pragma once

#include <ui/paint/Mesh.h>
#include <ui/paint/RenderInterface.h>
#include <ui/base/Types.h>
struct ComparisonResult {
	bool skipped = true;
	bool success = false;
	bool is_equal = false;
	double similarity_score = 0;
	size_t absolute_difference_sum = 0;
	size_t max_absolute_difference_single_pixel = 0;
	ui::String error_msg;
};

struct TextureGeometry {
	ui::TextureHandle texture_handle = 0;
	ui::CompiledGeometryHandle geometry_handle = 0;
	ui::Mesh mesh;
};

bool CaptureScreenshot(const ui::String& filename, int clip_width);

ComparisonResult CompareScreenToPreviousCapture(ui::RenderInterface* render_interface, const ui::String& filename, TextureGeometry* out_reference,
	TextureGeometry* out_highlight);

void RenderTextureGeometry(ui::RenderInterface* render_interface, TextureGeometry& geometry);

void ReleaseTextureGeometry(ui::RenderInterface* render_interface, TextureGeometry& geometry);
