#pragma once

#include <ui/paint/CompiledFilterShader.h>
#include <ui/base/Header.h>
#include <ui/paint/Mesh.h>
#include <ui/paint/Texture.h>
#include <ui/paint/UniqueRenderResource.h>

namespace ui {

class RenderManager;

/**
    A representation of geometry to be rendered through its underlying render interface.

    A unique resource constructed through the render manager.
 */
class UI_CORE_API Geometry final : public UniqueRenderResource<Geometry, StableVectorIndex, StableVectorIndex::Invalid> {
public:
	enum class ReleaseMode { ReturnMesh, ClearMesh };

	Geometry() = default;

	void Render(Vector2f translation, Texture texture = {}, const CompiledShader& shader = {}) const;

	Mesh Release(ReleaseMode mode = ReleaseMode::ReturnMesh);

	const Mesh& GetMesh() const;

private:
	Geometry(RenderManager* render_manager, StableVectorIndex resource_handle);
	friend class RenderManager;
};

} // namespace ui
