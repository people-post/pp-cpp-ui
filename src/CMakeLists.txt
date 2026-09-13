# Owned engine: one OBJECT library per module folder (folded into STATIC ui_core), umbrella ui_core for consumers.

include("${CMAKE_SOURCE_DIR}/cmake/engine/Utilities.cmake")
include("${CMAKE_SOURCE_DIR}/cmake/engine/UiModule.cmake")

# Layer order matches ADR 002 (low → high). Optional svg before core so the umbrella can link it.
add_subdirectory(base)
add_subdirectory(paint)
add_subdirectory(style)
add_subdirectory(layout)
add_subdirectory(font)
add_subdirectory(dom)
add_subdirectory(xml)
add_subdirectory(data)
add_subdirectory(widgets)

if(UI_SVG_PLUGIN)
	add_subdirectory(svg)
endif()

add_subdirectory(core)
add_subdirectory(debugger)
