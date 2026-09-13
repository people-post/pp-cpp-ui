#[[
	Note: ui_shell is NOT a sample. It's a utility static library with code common to the pp-cpp-ui samples.
]]

add_library(ui_shell STATIC
	src/PlatformExtensions.cpp
	src/RendererExtensions.cpp
	src/Shell.cpp
	src/ShellFileInterface.cpp

	include/PlatformExtensions.h
	include/RendererExtensions.h
	include/Shell.h
	include/ShellFileInterface.h
)

set_common_target_options(ui_shell)

target_link_libraries(ui_shell PRIVATE ui_backend_${UI_BACKEND})

target_include_directories(ui_shell PUBLIC "include" "${CMAKE_SOURCE_DIR}/tests/support/reference_backends/all")

target_compile_definitions(ui_shell PRIVATE "UI_SAMPLES_ROOT=\"${CMAKE_SOURCE_DIR}/tests/support\"")

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	target_link_libraries(ui_shell PRIVATE Windows::Shell::LightweightUtility)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	target_link_libraries(ui_shell PRIVATE macOS::Cocoa)
endif()

target_link_libraries(ui_shell PUBLIC ui_core ui_debugger)

if(UI_BACKEND MATCHES "GL2$")
	target_compile_definitions(ui_shell PRIVATE "UI_RENDERER_GL2")
endif()
if(UI_BACKEND MATCHES "GL3$")
	target_compile_definitions(ui_shell PRIVATE "UI_RENDERER_GL3")
endif()
