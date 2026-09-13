#pragma once

#include <ui/Core/Types.h>

namespace ui {

namespace LayoutPools {

	void Initialize();
	void Shutdown();

	void* AllocateLayoutChunk(size_t size);
	void DeallocateLayoutChunk(void* chunk, size_t size);

} // namespace LayoutPools

} // namespace ui
