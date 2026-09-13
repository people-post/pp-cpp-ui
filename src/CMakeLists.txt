# Owned engine sources: core (+ svg) + debugger.

add_subdirectory(core)

if(UI_SVG_PLUGIN)
  add_subdirectory(svg)
endif()

add_subdirectory(debugger)
