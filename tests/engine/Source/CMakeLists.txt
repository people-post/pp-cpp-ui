add_subdirectory("Common")

if(UI_BENCHMARKS)
	add_subdirectory("Benchmarks")
endif()

add_subdirectory("UnitTests")

if(UI_VISUAL_TESTS)
	add_subdirectory("VisualTests")
endif()
