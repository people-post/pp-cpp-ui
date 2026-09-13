# Test-only reference backend: SDL + OpenGL 3 (forced by PpUiEngine when tests are on).

add_library(rmlui_backend_common_headers INTERFACE)
target_include_directories(rmlui_backend_common_headers INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
target_sources(rmlui_backend_common_headers INTERFACE
	"${CMAKE_CURRENT_LIST_DIR}/RmlUi_Backend.h"
)

if(RMLUI_BACKEND_SIMULATE_TOUCH)
	target_compile_definitions(rmlui_backend_common_headers INTERFACE "RMLUI_BACKEND_SIMULATE_TOUCH")
endif()

add_library(rmlui_backend_SDL_GL3 INTERFACE)
target_sources(rmlui_backend_SDL_GL3 INTERFACE
	"${CMAKE_CURRENT_LIST_DIR}/RmlUi_Platform_SDL.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/RmlUi_Renderer_GL3.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/RmlUi_Backend_SDL_GL3.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/RmlUi_Platform_SDL.h"
	"${CMAKE_CURRENT_LIST_DIR}/RmlUi_Renderer_GL3.h"
	"${CMAKE_CURRENT_LIST_DIR}/RmlUi_Include_GL3.h"
)
target_link_libraries(rmlui_backend_SDL_GL3 INTERFACE rmlui_backend_common_headers SDL::SDL SDL_image::SDL_image)
if(UNIX)
	target_link_libraries(rmlui_backend_SDL_GL3 INTERFACE ${CMAKE_DL_LIBS})
endif()
if(EMSCRIPTEN)
	target_link_libraries(rmlui_backend_SDL_GL3 INTERFACE OpenGL::GL)
endif()
