# Module library: ui_svg
ui_add_module(svg DEPENDS ui_widgets)

# SVG plugin library ui_svg (linked into umbrella ui_core when enabled).

target_sources(ui_svg PRIVATE
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

target_compile_definitions(ui_svg PUBLIC "UI_SVG_PLUGIN")

target_link_libraries(ui_svg PUBLIC lunasvg::lunasvg)
