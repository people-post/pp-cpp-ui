set(TARGET_NAME "ui_unit_tests")

add_executable(${TARGET_NAME}
	Animation.cpp
	ClickRouting.cpp
	Core.cpp
	DataBinding.cpp
	DataExpression.cpp
	DataModel.cpp
	Debugger.cpp
	Decorator.cpp
	Element.cpp
	ElementBackgroundBorder.cpp
	ElementDocument.cpp
	ElementHandle.cpp
	ElementFormControlSelect.cpp
	ElementImage.cpp
	ElementStyle.cpp
	EventListener.cpp
	Filter.cpp
	FlexFormatting.cpp
	Layout.cpp
	Localization.cpp
	main.cpp
	Math.cpp
	MediaQuery.cpp
	Properties.cpp
	PropertySpecification.cpp
	Selectors.cpp
	Specificity_Basic.cpp
	Specificity_MediaQuery.cpp
	StableVector.cpp
	StringUtilities.cpp
	StyleSheetParser.cpp
	Template.cpp
	URL.cpp
	Variant.cpp
	XMLParser.cpp
)

set_common_target_options(${TARGET_NAME})

target_include_directories(${TARGET_NAME} PRIVATE
	"${CMAKE_SOURCE_DIR}/src/base"
	"${CMAKE_SOURCE_DIR}/src/style"
	"${CMAKE_SOURCE_DIR}/src/layout"
	"${CMAKE_SOURCE_DIR}/src/dom"
	"${CMAKE_SOURCE_DIR}/src/text"
	"${CMAKE_SOURCE_DIR}/src/xml"
	"${CMAKE_SOURCE_DIR}/src/data"
	"${CMAKE_SOURCE_DIR}/src/paint"
	"${CMAKE_SOURCE_DIR}/src/widgets"
	"${CMAKE_SOURCE_DIR}/src/core"
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
