# Module library: ui_style
ui_add_module(style DEPENDS ui_paint)

# Auto-maintained sources for ui_style.
target_sources(ui_style PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/ComputeProperty.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/ComputeProperty.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/ConvolutionFilter.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Decorator.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DecoratorUtilities.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/DecoratorUtilities.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/EffectSpecification.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Filter.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertiesIterator.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertiesIteratorView.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Property.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyDefinition.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyDictionary.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserAnimation.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserAnimation.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserBoxShadow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserBoxShadow.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserColorStopList.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserColorStopList.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserColour.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserColour.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserDecorator.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserFilter.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserFontEffect.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserKeyword.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserKeyword.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserNumber.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserNumber.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserRatio.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserRatio.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserString.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserString.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserTransform.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyParserTransform.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertyShorthandDefinition.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/PropertySpecification.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Spritesheet.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/StyleSheetSpecification.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Transform.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/TransformState.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/TransformState.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/TransformUtilities.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/TypeConverterStyle.cpp"
)

if(PP_UI_BUILD_TESTS OR UI_TESTS)
	add_subdirectory(tests)
endif()
