#pragma once

#include <ui/Core/Debug.h>
#include <ui/Core/Traits.h>

namespace ui {

template <typename T>
class ControlledLifetimeResource : NonCopyMoveable {
public:
	ControlledLifetimeResource() = default;
	~ControlledLifetimeResource() noexcept
	{
#if defined(UI_PLATFORM_WIN32) && !defined(UI_STATIC_LIB)
		UI_ASSERTMSG(!pointer || intentionally_leaked, "Resource was not properly shut down.");
#endif
	}

	explicit operator bool() const noexcept { return pointer != nullptr; }

	void Initialize()
	{
		UI_ASSERTMSG(!pointer, "Resource already initialized.");
		pointer = new T();
	}

	void InitializeIfEmpty()
	{
		if (!pointer)
			Initialize();
		else
			SetIntentionallyLeaked(false);
	}

	void Leak() { SetIntentionallyLeaked(true); }

	void Shutdown()
	{
		UI_ASSERTMSG(pointer, "Shutting down resource that was not initialized, or has been shut down already.");
		UI_ASSERTMSG(!intentionally_leaked, "Shutting down resource that was marked as leaked.");
		delete pointer;
		pointer = nullptr;
	}

	T* operator->()
	{
		UI_ASSERTMSG(pointer, "Resource used before it was initialized, or after it was shut down.");
		return pointer;
	}

private:
#ifdef UI_DEBUG
	void SetIntentionallyLeaked(bool leaked) { intentionally_leaked = leaked; }
	bool intentionally_leaked = false;
#else
	void SetIntentionallyLeaked(bool /*leaked*/) {}
#endif

	T* pointer = nullptr;
};

} // namespace ui
