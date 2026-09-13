#pragma once

#include <ui/paint/Mesh.h>
#include <ui/paint/RenderInterface.h>
#include <ui/base/SystemInterface.h>
#include <Shell.h>

class TestsSystemInterface : public ui::SystemInterface {
public:
	~TestsSystemInterface();

	double GetElapsedTime() override;

	bool LogMessage(ui::Log::Type type, const ui::String& message) override;

	// Checks and clears previously logged messages, then sets the number of expected
	// warnings and errors until the next call.
	void SetNumExpectedWarnings(int num_expected_warnings);

	void SetManualTime(double t);

	void Reset();

private:
	bool manual_time = false;
	double elapsed_time = 0.0;

	int num_logged_warnings = 0;
	int num_expected_warnings = 0;

	ui::StringList warnings;
};

class TestsRenderInterface : public ui::RenderInterface {
public:
	struct Counters {
		size_t compile_geometry;
		size_t render_geometry;
		size_t release_geometry;
		size_t load_texture;
		size_t generate_texture;
		size_t release_texture;
		size_t enable_scissor;
		size_t set_scissor;
		size_t enable_clip_mask;
		size_t render_to_clip_mask;
		size_t set_transform;
		size_t compile_filter;
		size_t release_filter;
		size_t compile_shader;
		size_t render_shader;
		size_t release_shader;
	};

	ui::CompiledGeometryHandle CompileGeometry(ui::Span<const ui::Vertex> vertices, ui::Span<const int> indices) override;
	void RenderGeometry(ui::CompiledGeometryHandle handle, ui::Vector2f translation, ui::TextureHandle texture) override;
	void ReleaseGeometry(ui::CompiledGeometryHandle handle) override;

	ui::TextureHandle LoadTexture(ui::Vector2i& texture_dimensions, const ui::String& source) override;
	ui::TextureHandle GenerateTexture(ui::Span<const ui::byte> source_data, ui::Vector2i source_dimensions) override;
	void ReleaseTexture(ui::TextureHandle texture_handle) override;

	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(ui::Rectanglei region) override;

	void EnableClipMask(bool enable) override;
	void RenderToClipMask(ui::ClipMaskOperation mask_operation, ui::CompiledGeometryHandle geometry, ui::Vector2f translation) override;

	void SetTransform(const ui::Matrix4f* transform) override;

	ui::CompiledFilterHandle CompileFilter(const ui::String& name, const ui::Dictionary& parameters) override;
	void ReleaseFilter(ui::CompiledFilterHandle filter) override;

	ui::CompiledShaderHandle CompileShader(const ui::String& name, const ui::Dictionary& parameters) override;
	void RenderShader(ui::CompiledShaderHandle shader, ui::CompiledGeometryHandle geometry, ui::Vector2f translation,
		ui::TextureHandle texture) override;
	void ReleaseShader(ui::CompiledShaderHandle shader) override;

	const Counters& GetCounters() const { return counters; }
	void ResetCounters();
	const Counters& GetCountersFromPreviousReset() const { return counters_from_previous_reset; }

	void ExpectCompileGeometry(ui::Vector<ui::Mesh> meshes);

	void Reset();

private:
	void VerifyMeshes();

	Counters counters = {};
	Counters counters_from_previous_reset = {};
	ui::Vector<ui::Mesh> meshes;
	bool meshes_set = false;
};
