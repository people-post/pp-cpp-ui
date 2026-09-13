set(TARGET_NAME "ui_unit_tests")

# Thin runner: modules register sources via ui_add_module_tests() from
# src/<module>/tests/CMakeLists.txt (collected on UI_UNIT_TEST_SOURCES).
get_property(UI_UNIT_TEST_SOURCES GLOBAL PROPERTY UI_UNIT_TEST_SOURCES)
if(NOT UI_UNIT_TEST_SOURCES)
	message(FATAL_ERROR "ui_unit_tests: no module test sources registered (UI_UNIT_TEST_SOURCES empty)")
endif()

add_executable(${TARGET_NAME}
	main.cpp
	${UI_UNIT_TEST_SOURCES}
)

set_common_target_options(${TARGET_NAME})

target_include_directories(${TARGET_NAME} PRIVATE
	"${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(${TARGET_NAME} PRIVATE
	ui_tests_common
	ui_core
	doctest::doctest
	trompeloeil::trompeloeil
)

if(NOT EMSCRIPTEN)
	doctest_discover_tests(${TARGET_NAME})
	add_test(NAME ui_unit_tests COMMAND ${TARGET_NAME})
endif()
