# Module library: ui_layout
ui_add_module(layout DEPENDS ui_style)

# Auto-maintained sources for ui_layout.
target_sources(ui_layout PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/BlockContainer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/BlockContainer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/BlockFormattingContext.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/BlockFormattingContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Box.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ContainerBox.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ContainerBox.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/FlexFormattingContext.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FlexFormattingContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/FloatedBoxSpace.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FloatedBoxSpace.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/FormattingContext.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FormattingContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/InlineBox.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/InlineBox.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/InlineContainer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/InlineContainer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/InlineLevelBox.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/InlineLevelBox.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/InlineTypes.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/LayoutBox.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/LayoutBox.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/LayoutDetails.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/LayoutDetails.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/LayoutEngine.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/LayoutEngine.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/LayoutPools.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/LayoutPools.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/LineBox.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/LineBox.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/ReplacedFormattingContext.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ReplacedFormattingContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/TableFormattingContext.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/TableFormattingContext.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/TableFormattingDetails.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/TableFormattingDetails.h"
)

if(PP_UI_BUILD_TESTS OR UI_TESTS)
	add_subdirectory(tests)
endif()
