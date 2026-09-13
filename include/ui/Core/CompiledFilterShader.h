#pragma once

#include "Header.h"
#include "UniqueRenderResource.h"

namespace ui {

class RenderManager;

/**
    A compiled filter to be applied when compositing layers in the render manager.

    Represents a unique render resource constructed through the render manager.
 */
class UI_CORE_API CompiledFilter final : public UniqueRenderResource<CompiledFilter, CompiledFilterHandle, CompiledFilterHandle(0)> {
public:
	CompiledFilter() = default;

	void AddHandleTo(FilterHandleList& list);

	void Release();

private:
	CompiledFilter(RenderManager* render_manager, CompiledFilterHandle resource_handle) : UniqueRenderResource(render_manager, resource_handle) {}
	friend class RenderManager;
};

/**
    A compiled shader to be used when rendering geometry.

    Represents a unique render resource constructed through the render manager.
 */
class UI_CORE_API CompiledShader final : public UniqueRenderResource<CompiledShader, CompiledShaderHandle, CompiledShaderHandle(0)> {
public:
	CompiledShader() = default;

	void Release();

private:
	CompiledShader(RenderManager* render_manager, CompiledShaderHandle resource_handle) : UniqueRenderResource(render_manager, resource_handle) {}
	friend class RenderManager;
};

} // namespace ui
