#pragma once

#include <ui/base/Types.h>

namespace ui {

namespace Utilities {

	template <class T>
	inline void HashCombine(size_t& seed, const T& v)
	{
		Hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

} // namespace Utilities
} // namespace ui
