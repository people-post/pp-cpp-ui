# Owned engine sources split by module. Single link target: ui_core.

# Create the library target first; modules append sources.
add_library(ui_core
	# bootstrap sources live in core/; added via add_subdirectory(core)
)

add_subdirectory(base)
add_subdirectory(style)
add_subdirectory(layout)
add_subdirectory(dom)
add_subdirectory(font)
add_subdirectory(xml)
add_subdirectory(data)
add_subdirectory(paint)
add_subdirectory(widgets)
add_subdirectory(core)

if(UI_SVG_PLUGIN)
  add_subdirectory(svg)
endif()

add_subdirectory(debugger)
