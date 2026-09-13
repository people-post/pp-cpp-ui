# Test-only reference backend: SDL + OpenGL 3 (forced by PpUiEngine when tests are on).

add_library(ui_backend_common_headers INTERFACE)
target_include_directories(ui_backend_common_headers INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
target_sources(ui_backend_common_headers INTERFACE
	"${CMAKE_CURRENT_LIST_DIR}/Ui_Backend.h"
)

if(UI_BACKEND_SIMULATE_TOUCH)
	target_compile_definitions(ui_backend_common_headers INTERFACE "UI_BACKEND_SIMULATE_TOUCH")
endif()

add_library(ui_backend_SDL_GL3 INTERFACE)
target_sources(ui_backend_SDL_GL3 INTERFACE
	"${CMAKE_CURRENT_LIST_DIR}/Ui_Platform_SDL.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/Ui_Renderer_GL3.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/Ui_Backend_SDL_GL3.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/Ui_Platform_SDL.h"
	"${CMAKE_CURRENT_LIST_DIR}/Ui_Renderer_GL3.h"
	"${CMAKE_CURRENT_LIST_DIR}/Ui_Include_GL3.h"
)
target_link_libraries(ui_backend_SDL_GL3 INTERFACE ui_backend_common_headers SDL::SDL SDL_image::SDL_image)
if(UNIX)
	target_link_libraries(ui_backend_SDL_GL3 INTERFACE ${CMAKE_DL_LIBS})
endif()
if(EMSCRIPTEN)
	target_link_libraries(ui_backend_SDL_GL3 INTERFACE OpenGL::GL)
endif()
