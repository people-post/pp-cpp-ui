# Module library: ui_xml
ui_add_module(xml DEPENDS ui_dom)

# Auto-maintained sources for ui_xml.
target_sources(ui_xml PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/BaseXMLParser.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FactoryXML.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DocumentHeader.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Template.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/XMLNodeHandler.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/XMLNodeHandlerBody.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/XMLNodeHandlerBody.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/XMLNodeHandlerDefault.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/XMLNodeHandlerDefault.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/XMLNodeHandlerHead.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/XMLNodeHandlerHead.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/XMLNodeHandlerTemplate.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/XMLNodeHandlerTemplate.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/XMLParseTools.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/XMLParseTools.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/XMLParser.cpp"
)

if(PP_UI_BUILD_TESTS OR UI_TESTS)
	add_subdirectory(tests)
endif()
