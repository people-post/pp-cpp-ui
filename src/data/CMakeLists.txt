# Module library: ui_data
ui_add_module(data DEPENDS ui_dom)

# Auto-maintained sources for ui_data.
target_sources(ui_data PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/ElementData.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ContextData.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ElementUtilitiesData.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FactoryData.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataController.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataController.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataControllerDefault.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataControllerDefault.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataExpression.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataExpression.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataModel.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataModel.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataModelHandle.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataTypeRegister.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataVariable.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataView.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataView.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataViewDefault.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DataViewDefault.h"
)

if(PP_UI_BUILD_TESTS OR UI_TESTS)
	add_subdirectory(tests)
endif()
