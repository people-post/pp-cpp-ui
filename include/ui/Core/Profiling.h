#pragma once

#ifdef UI_TRACY_PROFILING

	#include <tracy/Tracy.hpp>

	#define UI_ZoneNamed(varname, active) ZoneNamed(varname, active)
	#define UI_ZoneNamedN(varname, name, active) ZoneNamedN(varname, name, active)
	#define UI_ZoneNamedC(varname, color, active) ZoneNamedC(varname, color, active)
	#define UI_ZoneNamedNC(varname, name, color, active) ZoneNamedNC(varname, name, color, active)

	#define UI_ZoneScoped ZoneScoped
	#define UI_ZoneScopedN(name) ZoneScopedN(name)
	#define UI_ZoneScopedC(color) ZoneScopedC(color)
	#define UI_ZoneScopedNC(name, color) ZoneScopedNC(name, color)

	#define UI_ZoneText(txt, size) ZoneText(txt, size)
	#define UI_ZoneName(txt, size) ZoneName(txt, size)

	#define UI_TracyPlot(name, val) TracyPlot(name, val)

	#define UI_FrameMark FrameMark
	#define UI_FrameMarkNamed(name) FrameMarkNamed(name)
	#define UI_FrameMarkStart(name) FrameMarkStart(name)
	#define UI_FrameMarkEnd(name) FrameMarkEnd(name)

#else

	#define UI_ZoneNamed(varname, active)
	#define UI_ZoneNamedN(varname, name, active)
	#define UI_ZoneNamedC(varname, color, active)
	#define UI_ZoneNamedNC(varname, name, color, active)

	#define UI_ZoneScoped
	#define UI_ZoneScopedN(name)
	#define UI_ZoneScopedC(color)
	#define UI_ZoneScopedNC(name, color)

	#define UI_ZoneText(txt, size)
	#define UI_ZoneName(txt, size)

	#define UI_TracyPlot(name, val)

	#define UI_FrameMark
	#define UI_FrameMarkNamed(name)
	#define UI_FrameMarkStart(name)
	#define UI_FrameMarkEnd(name)

#endif
