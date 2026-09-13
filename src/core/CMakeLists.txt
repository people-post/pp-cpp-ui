# Composition root: umbrella STATIC library consumers link (ui::core / pp_ui_core).
# Module implementations live in ui_base…ui_widgets (and optional ui_svg).

add_library(ui_core STATIC)
add_library(ui::core ALIAS ui_core)

# Compatibility aliases expected by pp-browser pins.
if(NOT TARGET RmlUi::Core)
	add_library(RmlUi::Core ALIAS ui_core)
endif()
if(NOT TARGET pp_ui_rml)
	add_library(pp_ui_rml INTERFACE)
	target_link_libraries(pp_ui_rml INTERFACE ui_core)
endif()

target_sources(ui_core PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/Core.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/RegisterDefaultFactories.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/RegisterDefaultFactories.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/FileInterfaceDefault.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FileInterfaceDefault.h"
)

set_common_target_options(ui_core)

target_include_directories(ui_core
	PUBLIC
		"$<BUILD_INTERFACE:${PP_UI_INCLUDE_DIR}>"
		"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/base/containers>"
		"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
	PRIVATE
		"${CMAKE_SOURCE_DIR}/src"
		"${PP_UI_INCLUDE_DIR}"
		"${CMAKE_SOURCE_DIR}/src/base/containers"
)

# Fold per-folder OBJECT libs into this umbrella archive (avoids static link cycles).
set(_ui_core_modules ui_base ui_paint ui_style ui_layout ui_font ui_dom ui_xml ui_data ui_widgets)
if(TARGET ui_svg)
	list(APPEND _ui_core_modules ui_svg)
endif()
foreach(_m IN LISTS _ui_core_modules)
	target_sources(ui_core PRIVATE $<TARGET_OBJECTS:${_m}>)
	# Propagate include dirs / compile defs from modules to consumers of ui_core.
	target_link_libraries(ui_core PUBLIC ${_m})
endforeach()
unset(_ui_core_modules)

set_target_properties(ui_core PROPERTIES
	EXPORT_NAME "Core"
	OUTPUT_NAME "ui"
)

set(UI_VERSION_SHORT "6.2")
target_compile_definitions(ui_core PRIVATE "UI_VERSION=\"${UI_VERSION_SHORT}\"")
target_compile_definitions(ui_core PUBLIC "UI_STATIC_LIB")

option(UI_DEBUG_SELECTION_HANDLES "Draw magenta/yellow debug markers at selection handle anchors" OFF)
if(UI_DEBUG_SELECTION_HANDLES)
	target_compile_definitions(ui_core PRIVATE UI_DEBUG_SELECTION_HANDLES)
endif()

if(UI_LOTTIE_PLUGIN)
	target_link_libraries(ui_core PRIVATE rlottie::rlottie)
endif()

if(UI_SVG_PLUGIN AND TARGET lunasvg::lunasvg)
	# Keep MSVC-friendly public propagation for the umbrella (svg module also links it).
	target_link_libraries(ui_core PUBLIC lunasvg::lunasvg)
endif()

if(UI_TRACY_PROFILING)
	if(CMAKE_CONFIGURATION_TYPES AND UI_TRACY_CONFIGURATION)
		target_link_libraries(ui_core PUBLIC "$<$<CONFIG:Tracy>:Tracy::TracyClient>")
		target_compile_definitions(ui_core PUBLIC "$<$<CONFIG:Tracy>:UI_TRACY_PROFILING>")
		if(UI_TRACY_MEMORY_PROFILING)
			target_compile_definitions(ui_core PRIVATE "$<$<CONFIG:Tracy>:UI_TRACY_MEMORY_PROFILING>")
		endif()
	else()
		target_link_libraries(ui_core PUBLIC Tracy::TracyClient)
		target_compile_definitions(ui_core PUBLIC "UI_TRACY_PROFILING")
		if(UI_TRACY_MEMORY_PROFILING)
			target_compile_definitions(ui_core PRIVATE "UI_TRACY_MEMORY_PROFILING")
		endif()
	endif()
endif()

if(NOT UI_THIRDPARTY_CONTAINERS)
	target_compile_definitions(ui_core PUBLIC "UI_NO_THIRDPARTY_CONTAINERS")
	message(STATUS "Disabling third-party containers for pp-cpp-ui.")
endif()

if(UI_CUSTOM_RTTI)
	target_compile_definitions(ui_core PUBLIC "UI_CUSTOM_RTTI")
endif()

if(UI_MATRIX_ROW_MAJOR)
	target_compile_definitions(ui_core PUBLIC "UI_MATRIX_ROW_MAJOR")
endif()

if(UI_PRECOMPILED_HEADERS AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
	target_precompile_headers(ui_core PRIVATE "${CMAKE_SOURCE_DIR}/src/base/precompiled.h")
elseif(UI_PRECOMPILED_HEADERS)
	message(STATUS "Could not enable precompiled headers, requires CMake version 3.16 or greater.")
endif()

if(UI_CUSTOM_CONFIGURATION AND UI_CUSTOM_CONFIGURATION_FILE)
	target_compile_definitions(ui_core PUBLIC "UI_CUSTOM_CONFIGURATION_FILE=\"${UI_CUSTOM_CONFIGURATION_FILE}\"")
endif()
if(UI_CUSTOM_CONFIGURATION AND UI_CUSTOM_INCLUDE_DIRS)
	target_include_directories(ui_core PUBLIC ${UI_CUSTOM_INCLUDE_DIRS})
endif()
if(UI_CUSTOM_CONFIGURATION AND UI_CUSTOM_LINK_LIBRARIES)
	target_link_libraries(ui_core PUBLIC ${UI_CUSTOM_LINK_LIBRARIES})
endif()

install(TARGETS ui_core
	EXPORT UiTargets
	${UI_RUNTIME_DEPENDENCY_SET_ARG}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install_target_pdb(ui_core)
