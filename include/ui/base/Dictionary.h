#pragma once

#include <ui/base/Header.h>
#include <ui/base/Variant.h>

namespace ui {

inline Variant* GetIf(Dictionary& dictionary, const String& key)
{
	auto it = dictionary.find(key);
	if (it != dictionary.end())
		return &(it->second);
	return nullptr;
}
inline const Variant* GetIf(const Dictionary& dictionary, const String& key)
{
	auto it = dictionary.find(key);
	if (it != dictionary.end())
		return &(it->second);
	return nullptr;
}
template <typename T>
inline T Get(const Dictionary& dictionary, const String& key, const T& default_value)
{
	T result = default_value;
	auto it = dictionary.find(key);
	if (it != dictionary.end())
		it->second.GetInto(result);
	return result;
}

} // namespace ui
