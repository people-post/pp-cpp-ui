#[[
  ui_add_module(<name> DEPENDS <ui_module_targets...>)

  Creates an OBJECT library ui_<name> (alias ui::<name>) so per-folder builds keep
  include DAG separation without static-archive cycles (layout↔dom, etc.).
  The umbrella ui_core consumes $<TARGET_OBJECTS:ui_*>.
]]

function(ui_add_module name)
	cmake_parse_arguments(ARG "" "" "DEPENDS" ${ARGN})
	set(target "ui_${name}")

	add_library(${target} OBJECT)
	add_library(ui::${name} ALIAS ${target})

	set_common_target_options(${target})

	# OBJECT libraries need POSITION_INDEPENDENT_CODE when linked into shared consumers.
	set_target_properties(${target} PROPERTIES
		POSITION_INDEPENDENT_CODE ON
		EXPORT_NAME "${name}"
	)

	target_include_directories(${target}
		PUBLIC
			"$<BUILD_INTERFACE:${PP_UI_INCLUDE_DIR}>"
			"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/base/containers>"
			"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
		PRIVATE
			"${CMAKE_SOURCE_DIR}/src"
			"${PP_UI_INCLUDE_DIR}"
			"${CMAKE_SOURCE_DIR}/src/base/containers"
	)

	target_compile_definitions(${target} PUBLIC "UI_STATIC_LIB")

	# Usage requirements only (no archive link cycles).
	if(ARG_DEPENDS)
		target_link_libraries(${target} PUBLIC ${ARG_DEPENDS})
	endif()

	if(UI_PRECOMPILED_HEADERS AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
		target_precompile_headers(${target} PRIVATE "${CMAKE_SOURCE_DIR}/src/base/precompiled.h")
	endif()
endfunction()
