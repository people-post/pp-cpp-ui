#pragma once

#include "Header.h"
#include "Texture.h"
#include "Vertex.h"

namespace ui {

struct UI_CORE_API Mesh {
	Vector<Vertex> vertices;
	Vector<int> indices;

	explicit operator bool() const { return !indices.empty(); }
	friend bool operator==(const Mesh& lhs, const Mesh& rhs) { return lhs.vertices == rhs.vertices && lhs.indices == rhs.indices; }
	friend bool operator!=(const Mesh& lhs, const Mesh& rhs) { return !(lhs == rhs); }
};

struct UI_CORE_API TexturedMesh {
	Mesh mesh;
	Texture texture;
};

using TexturedMeshList = Vector<TexturedMesh>;

} // namespace ui
