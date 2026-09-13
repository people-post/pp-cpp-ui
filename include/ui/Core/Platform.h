#pragma once

#if defined __WIN32__ || defined _WIN32
	#define UI_PLATFORM_WIN32
	#define UI_PLATFORM_NAME "win32"
#elif defined __APPLE_CC__
	#define UI_PLATFORM_UNIX
	#define UI_PLATFORM_MACOSX
	#define UI_PLATFORM_NAME "macosx"
#elif defined __EMSCRIPTEN__
	#define UI_PLATFORM_UNIX
	#define UI_PLATFORM_EMSCRIPTEN
	#define UI_PLATFORM_NAME "emscripten"
#else
	#define UI_PLATFORM_UNIX
	#define UI_PLATFORM_LINUX
	#define UI_PLATFORM_NAME "linux"
#endif

#if !defined NDEBUG && !defined UI_DEBUG
	#define UI_DEBUG
#endif

#if defined __LP64__ || defined _M_X64 || defined _WIN64 || defined __MINGW64__ || defined _LP64
	#define UI_ARCH_64
#else
	#define UI_ARCH_32
#endif

#if defined(UI_PLATFORM_WIN32) && !defined(__MINGW32__)
	#define UI_PLATFORM_WIN32_NATIVE

	// declaration of 'identifier' hides class member
	#pragma warning(disable : 4458)

	// <type> needs to have dll-interface to be used by clients
	#pragma warning(disable : 4251)

	// <function> was declared deprecated
	#pragma warning(disable : 4996)
#endif

// Tell the compiler of printf-like functions, warns on incorrect usage.
#if defined __MINGW32__
	#define UI_ATTRIBUTE_FORMAT_PRINTF(i, f) __attribute__((format(__MINGW_PRINTF_FORMAT, i, f)))
#elif defined __GNUC__ || defined __clang__
	#define UI_ATTRIBUTE_FORMAT_PRINTF(i, f) __attribute__((format(printf, i, f)))
#else
	#define UI_ATTRIBUTE_FORMAT_PRINTF(i, f)
#endif
