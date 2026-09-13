#pragma once

#include <ui/Core/Core.h>
#include <ui/Core/RenderManager.h>
#include <ui/Core/Types.h>

namespace ui {

class CompiledFilter;
class CompiledShader;
class CallbackTexture;
class Geometry;
class Texture;

class RenderManagerAccess {
private:
	template <typename T>
	static auto ReleaseResource(RenderManager* render_manager, T& resource)
	{
		return render_manager->ReleaseResource(resource);
	}

	static Vector2i GetDimensions(RenderManager* render_manager, TextureFileIndex texture);
	static Vector2i GetDimensions(RenderManager* render_manager, StableVectorIndex callback_texture);

	static void Render(RenderManager* render_manager, const Geometry& geometry, Vector2f translation, Texture texture, const CompiledShader& shader);

	static void GetTextureSourceList(RenderManager* render_manager, StringList& source_list);
	static const Mesh& GetMesh(RenderManager* render_manager, const Geometry& geometry);

	static bool ReleaseTexture(RenderManager* render_manager, const String& texture_source);
	static void ReleaseAllTextures(RenderManager* render_manager);
	static void ReleaseAllCompiledGeometry(RenderManager* render_manager);

	friend class CompiledFilter;
	friend class CompiledShader;
	friend class CallbackTexture;
	friend class Geometry;
	friend class Texture;

	friend StringList ui::GetTextureSourceList();
	friend bool ui::ReleaseTexture(const String&, RenderInterface*);
	friend void ui::ReleaseTextures(RenderInterface*);
	friend void ui::ReleaseCompiledGeometry(RenderInterface*);
	friend void ui::ReleaseRenderManagers();
};

} // namespace ui
