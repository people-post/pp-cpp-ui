set(TARGET_NAME "ui_unit_tests")

# Unit tests live next to their module under src/<module>/tests/*_test.cpp.
# This target remains the single doctest runner; harness stays under tests/.
set(UI_SRC "${CMAKE_SOURCE_DIR}/src")

add_executable(${TARGET_NAME}
	main.cpp

	# base
	"${UI_SRC}/base/tests/Math_test.cpp"
	"${UI_SRC}/base/tests/StableVector_test.cpp"
	"${UI_SRC}/base/tests/StringUtilities_test.cpp"
	"${UI_SRC}/base/tests/URL_test.cpp"
	"${UI_SRC}/base/tests/Variant_test.cpp"

	# core
	"${UI_SRC}/core/tests/Core_test.cpp"

	# data
	"${UI_SRC}/data/tests/DataBinding_test.cpp"
	"${UI_SRC}/data/tests/DataExpression_test.cpp"
	"${UI_SRC}/data/tests/DataModel_test.cpp"

	# debugger
	"${UI_SRC}/debugger/tests/Debugger_test.cpp"

	# dom
	"${UI_SRC}/dom/tests/Animation_test.cpp"
	"${UI_SRC}/dom/tests/ClickRouting_test.cpp"
	"${UI_SRC}/dom/tests/Decorator_test.cpp"
	"${UI_SRC}/dom/tests/Element_test.cpp"
	"${UI_SRC}/dom/tests/ElementBackgroundBorder_test.cpp"
	"${UI_SRC}/dom/tests/ElementDocument_test.cpp"
	"${UI_SRC}/dom/tests/ElementHandle_test.cpp"
	"${UI_SRC}/dom/tests/ElementStyle_test.cpp"
	"${UI_SRC}/dom/tests/EventListener_test.cpp"
	"${UI_SRC}/dom/tests/Filter_test.cpp"
	"${UI_SRC}/dom/tests/Localization_test.cpp"
	"${UI_SRC}/dom/tests/MediaQuery_test.cpp"
	"${UI_SRC}/dom/tests/Selectors_test.cpp"
	"${UI_SRC}/dom/tests/Specificity_Basic_test.cpp"
	"${UI_SRC}/dom/tests/Specificity_MediaQuery_test.cpp"
	"${UI_SRC}/dom/tests/Template_test.cpp"

	# layout
	"${UI_SRC}/layout/tests/FlexFormatting_test.cpp"
	"${UI_SRC}/layout/tests/Layout_test.cpp"

	# style
	"${UI_SRC}/style/tests/Properties_test.cpp"
	"${UI_SRC}/style/tests/PropertySpecification_test.cpp"
	"${UI_SRC}/style/tests/StyleSheetParser_test.cpp"

	# widgets
	"${UI_SRC}/widgets/tests/ElementFormControlSelect_test.cpp"
	"${UI_SRC}/widgets/tests/ElementImage_test.cpp"

	# xml
	"${UI_SRC}/xml/tests/XMLParser_test.cpp"
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
