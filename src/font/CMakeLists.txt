# Module library: ui_font
ui_add_module(font DEPENDS ui_layout)

# Auto-maintained sources for ui_font.
target_sources(ui_font PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/FontEffect.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FontEffectBlur.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FontEffectBlur.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/FontEffectGlow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FontEffectGlow.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/FontEffectInstancer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FontEffectOutline.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FontEffectOutline.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/FontEffectShadow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FontEffectShadow.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/FontEngineInterface.cpp"
)

if(UI_FONT_ENGINE STREQUAL "freetype")
	add_subdirectory(default)
	if(UI_FONT_ENGINE_HARFBUZZ)
		add_subdirectory(harfbuzz)
		target_link_libraries(ui_font PUBLIC harfbuzz::harfbuzz)
	endif()
	target_link_libraries(ui_font PUBLIC Freetype::Freetype)
endif()
