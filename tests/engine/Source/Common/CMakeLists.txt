add_library(ui_tests_common STATIC
	Mocks.h
	TestsInterface.cpp
	TestsInterface.h
	TestsShell.cpp
	TestsShell.h
	TypesToString.h
)

set_common_target_options(ui_tests_common)

target_include_directories(ui_tests_common INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(ui_tests_common PUBLIC
	ui_core
	ui_shell
	doctest::doctest
	trompeloeil::trompeloeil
)
