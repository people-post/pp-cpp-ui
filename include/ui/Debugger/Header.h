#pragma once

#include <ui/base/Platform.h>

#if !defined UI_STATIC_LIB
	#ifdef UI_PLATFORM_WIN32
		#ifdef UI_DEBUGGER_EXPORTS
			#define UI_DEBUGGER_API __declspec(dllexport)
		#else
			#define UI_DEBUGGER_API __declspec(dllimport)
		#endif
	#else
		#define UI_DEBUGGER_API __attribute__((visibility("default")))
	#endif
#else
	#define UI_DEBUGGER_API
#endif
