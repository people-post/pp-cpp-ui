# pp-cpp-ui ships a single test/sample backend: SDL + OpenGL 3.
if(NOT UI_BACKEND OR UI_BACKEND STREQUAL "auto" OR UI_BACKEND STREQUAL "native")
	set(UI_BACKEND SDL_GL3 CACHE STRING "UI backend" FORCE)
endif()
if(NOT UI_BACKEND STREQUAL "SDL_GL3")
	message(FATAL_ERROR
		"pp-cpp-ui only supports UI_BACKEND=SDL_GL3 (got '${UI_BACKEND}').")
endif()
message(STATUS "Using pp-cpp-ui backend ${UI_BACKEND}")
