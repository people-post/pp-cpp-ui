#pragma once

#include "Header.h"

// Define for breakpointing.
#if defined(UI_PLATFORM_WIN32)
	#if defined(__MINGW32__)
		#define UI_BREAK       \
			{                     \
				asm("int $0x03"); \
			}
	#elif defined(_MSC_VER)
		#define UI_BREAK     \
			{                   \
				__debugbreak(); \
			}
	#else
		#define UI_BREAK
	#endif
#elif defined(UI_PLATFORM_LINUX)
	#if defined __GNUC__
		#define UI_BREAK       \
			{                     \
				__builtin_trap(); \
			}
	#else
		#define UI_BREAK
	#endif
#elif defined(UI_PLATFORM_MACOSX)
	#define UI_BREAK       \
		{                     \
			__builtin_trap(); \
		}
#else
	#define UI_BREAK
#endif

namespace ui {

bool UI_CORE_API Assert(const char* message, const char* file, int line);

}

// Define the pp-cpp-ui assertion macros.
#if !defined UI_DEBUG

	#define UI_ASSERT(x)
	#define UI_ASSERTMSG(x, m)
	#define UI_ERROR
	#define UI_ERRORMSG(m)
	#define UI_VERIFY(x) x
	#define UI_ASSERT_NONRECURSIVE

#else

	#define UI_ASSERT(x)                                                   \
		if (!(x))                                                             \
		{                                                                     \
			if (!(::ui::Assert("UI_ASSERT(" #x ")", __FILE__, __LINE__))) \
			{                                                                 \
				UI_BREAK;                                                  \
			}                                                                 \
		}
	#define UI_ASSERTMSG(x, m)                        \
		if (!(x))                                        \
		{                                                \
			if (!(::ui::Assert(m, __FILE__, __LINE__))) \
			{                                            \
				UI_BREAK;                             \
			}                                            \
		}
	#define UI_ERROR                                          \
		if (!(::ui::Assert("UI_ERROR", __FILE__, __LINE__))) \
		{                                                        \
			UI_BREAK;                                         \
		}
	#define UI_ERRORMSG(m)                        \
		if (!(::ui::Assert(m, __FILE__, __LINE__))) \
		{                                            \
			UI_BREAK;                             \
		}
	#define UI_VERIFY(x) UI_ASSERT(x)

struct UiAssertNonrecursive {
	bool& entered;
	UiAssertNonrecursive(bool& entered) : entered(entered)
	{
		UI_ASSERTMSG(!entered, "A method defined as non-recursive was entered twice!");
		entered = true;
	}
	~UiAssertNonrecursive() { entered = false; }
};

	#define UI_ASSERT_NONRECURSIVE                   \
		static bool ui_nonrecursive_entered = false; \
		UiAssertNonrecursive ui_nonrecursive(ui_nonrecursive_entered)

#endif // UI_DEBUG
