#pragma once

#include <ui/Core/RenderInterface.h>
#include <ui/Core/Types.h>
#include <bitset>

enum class ProgramId;
enum class UniformId;
class RenderLayerStack;
namespace Gfx {
struct ProgramData;
struct FramebufferData;
} // namespace Gfx

class RenderInterface_GL3 : public ui::RenderInterface {
public:
	RenderInterface_GL3();
	~RenderInterface_GL3();

	// Returns true if the renderer was successfully constructed.
	explicit operator bool() const { return static_cast<bool>(program_data); }

	// The viewport should be updated whenever the window size changes.
	void SetViewport(int viewport_width, int viewport_height, int viewport_offset_x = 0, int viewport_offset_y = 0);

	// Sets up OpenGL states for taking rendering commands from pp-cpp-ui.
	void BeginFrame();
	// Draws the result to the backbuffer and restores OpenGL state.
	void EndFrame();

	// Optional, can be used to clear the active framebuffer.
	void Clear();

	// iOS (and some GLES platforms) use a non-zero window framebuffer; desktop GL uses 0.
	void SetOutputFramebuffer(unsigned int framebuffer_id);

	// Rebuild shaders, layer FBOs, and internal geometry after an EGL/GL context loss.
	void RecoverGpuResources();

	// -- Inherited from ui::RenderInterface --

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

	ui::LayerHandle PushLayer() override;
	void CompositeLayers(ui::LayerHandle source, ui::LayerHandle destination, ui::BlendMode blend_mode,
		ui::Span<const ui::CompiledFilterHandle> filters) override;
	void PopLayer() override;

	ui::TextureHandle SaveLayerAsTexture() override;

	ui::CompiledFilterHandle SaveLayerAsMaskImage() override;

	ui::CompiledFilterHandle CompileFilter(const ui::String& name, const ui::Dictionary& parameters) override;
	void ReleaseFilter(ui::CompiledFilterHandle filter) override;

	ui::CompiledShaderHandle CompileShader(const ui::String& name, const ui::Dictionary& parameters) override;
	void RenderShader(ui::CompiledShaderHandle shader_handle, ui::CompiledGeometryHandle geometry_handle, ui::Vector2f translation,
		ui::TextureHandle texture) override;
	void ReleaseShader(ui::CompiledShaderHandle effect_handle) override;

	// Can be passed to RenderGeometry() to enable texture rendering without changing the bound texture.
	static constexpr ui::TextureHandle TextureEnableWithoutBinding = ui::TextureHandle(-1);
	// Can be passed to RenderGeometry() to leave the bound texture and used program unchanged.
	static constexpr ui::TextureHandle TexturePostprocess = ui::TextureHandle(-2);

	// -- Utility functions for clients --

	const ui::Matrix4f& GetTransform() const;
	void ResetProgram();

	int GetViewportWidth() const { return viewport_width; }
	int GetViewportHeight() const { return viewport_height; }
	void BlitTopLayerRegion(ui::Rectanglei src_region_top_left, unsigned int dest_framebuffer, int dest_width, int dest_height);
	void BindTopLayerFramebuffer();

private:
	void UseProgram(ProgramId program_id);
	int GetUniformLocation(UniformId uniform_id) const;
	void SubmitTransformUniform(ui::Vector2f translation);

	void BlitLayerToPostprocessPrimary(ui::LayerHandle layer_handle);
	void RenderFilters(ui::Span<const ui::CompiledFilterHandle> filter_handles);

	void SetScissor(ui::Rectanglei region, bool vertically_flip = false);

	void DrawFullscreenQuad();
	void DrawFullscreenQuad(ui::Vector2f uv_offset, ui::Vector2f uv_scaling = ui::Vector2f(1.f));

	void RenderBlur(float sigma, const Gfx::FramebufferData& source_destination, const Gfx::FramebufferData& temp, ui::Rectanglei window_flipped);

	static constexpr size_t MaxNumPrograms = 32;
	std::bitset<MaxNumPrograms> program_transform_dirty;

	ui::Matrix4f transform;
	ui::Matrix4f projection;

	ProgramId active_program = {};
	ui::Rectanglei scissor_state;

	int viewport_width = 0;
	int viewport_height = 0;
	int viewport_offset_x = 0;
	int viewport_offset_y = 0;
	// Desktop GL: 0. iOS UIKit GLES: non-zero drawable FBO from SDL.
	unsigned int output_framebuffer = 0;

	ui::CompiledGeometryHandle fullscreen_quad_geometry = {};

	ui::UniquePtr<const Gfx::ProgramData> program_data;

	/*
	    Manages render targets, including the layer stack and postprocessing framebuffers.

	    Layers can be pushed and popped, creating new framebuffers as needed. Typically, geometry is rendered to the top
	    layer. The layer framebuffers may have MSAA enabled.

	    Postprocessing framebuffers are separate from the layers, and are commonly used to apply texture-wide effects
	    such as filters. They are used both as input and output during rendering, and do not use MSAA.
	*/
	class RenderLayerStack {
	public:
		RenderLayerStack();
		~RenderLayerStack();

		// Push a new layer. All references to previously retrieved layers are invalidated.
		ui::LayerHandle PushLayer();

		// Pop the top layer. All references to previously retrieved layers are invalidated.
		void PopLayer();

		const Gfx::FramebufferData& GetLayer(ui::LayerHandle layer) const;
		const Gfx::FramebufferData& GetTopLayer() const;
		ui::LayerHandle GetTopLayerHandle() const;

		const Gfx::FramebufferData& GetPostprocessPrimary() { return EnsureFramebufferPostprocess(0); }
		const Gfx::FramebufferData& GetPostprocessSecondary() { return EnsureFramebufferPostprocess(1); }
		const Gfx::FramebufferData& GetPostprocessTertiary() { return EnsureFramebufferPostprocess(2); }
		const Gfx::FramebufferData& GetBlendMask() { return EnsureFramebufferPostprocess(3); }

		void SwapPostprocessPrimarySecondary();

		void BeginFrame(int new_width, int new_height);
		void EndFrame();
		// Drop all layer/postprocess FBOs so the next BeginFrame recreates them.
		void InvalidateFramebuffers();

	private:
		void DestroyFramebuffers();
		const Gfx::FramebufferData& EnsureFramebufferPostprocess(int index);

		int width = 0, height = 0;

		// The number of active layers is manually tracked since we re-use the framebuffers stored in the fb_layers stack.
		int layers_size = 0;

		ui::Vector<Gfx::FramebufferData> fb_layers;
		ui::Vector<Gfx::FramebufferData> fb_postprocess;
	};

	RenderLayerStack render_layers;

	struct GLStateBackup {
		bool enable_cull_face;
		bool enable_blend;
		bool enable_stencil_test;
		bool enable_scissor_test;
		bool enable_depth_test;

		int viewport[4];
		int scissor[4];

		int active_texture;

		int stencil_clear_value;
		float color_clear_value[4];
		unsigned char color_writemask[4];

		int blend_equation_rgb;
		int blend_equation_alpha;
		int blend_src_rgb;
		int blend_dst_rgb;
		int blend_src_alpha;
		int blend_dst_alpha;

		struct Stencil {
			int func;
			int ref;
			int value_mask;
			int writemask;
			int fail;
			int pass_depth_fail;
			int pass_depth_pass;
		};
		Stencil stencil_front;
		Stencil stencil_back;
	};
	GLStateBackup glstate_backup = {};
};

/**
    Helper functions for the OpenGL 3 renderer.
 */
namespace RmlGL3 {

// Loads OpenGL functions. Optionally, the out message describes the loaded GL version or an error message on failure.
bool Initialize(ui::String* out_message = nullptr);

// Unloads OpenGL functions.
void Shutdown();

} // namespace RmlGL3
