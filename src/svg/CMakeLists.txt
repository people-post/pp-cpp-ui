# This plugin targets ui_core directly and is incorporated into the core library when enabled.

target_sources(ui_core PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/DecoratorSVG.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DecoratorSVG.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/ElementSVG.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/SVGCache.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/SVGCache.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/SVGPlugin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/SVGPlugin.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/XMLNodeHandlerSVG.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/XMLNodeHandlerSVG.h"
	"${PP_UI_INCLUDE_DIR}/ui/svg/ElementSVG.h"
)

target_compile_definitions(ui_core PRIVATE "UI_SVG_PLUGIN")
