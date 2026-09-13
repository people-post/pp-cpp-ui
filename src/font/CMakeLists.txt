# Auto-maintained module sources for ui_core.
target_sources(ui_core PRIVATE
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
		target_link_libraries(ui_core PRIVATE harfbuzz::harfbuzz)
	endif()
	target_link_libraries(ui_core PRIVATE Freetype::Freetype)
endif()
