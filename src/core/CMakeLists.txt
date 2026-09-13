# Declare RmlUi core library
# Not explicitly setting library type so that it can be chosen by consumer using BUILD_SHARED_LIBS. Header files are not
# necessary, but are included to improve navigation and code completion on IDEs and language servers.
add_library(rmlui_core
	BaseXMLParser.cpp
	Box.cpp
	BoxShadowCache.h
	BoxShadowCache.cpp
	BoxShadowHash.h
	CallbackTexture.cpp
	Clock.cpp
	Clock.h
	ClickRouting.cpp
	ClickRouting.h
	CompiledFilterShader.cpp
	ComputedValues.cpp
	ComputeProperty.cpp
	ComputeProperty.h
	Context.cpp
	SelectionContentBuilder.cpp
	SelectionController.cpp
	SelectionHighlight.cpp
	ContextInstancer.cpp
	ContextInstancerDefault.cpp
	ContextInstancerDefault.h
	ControlledLifetimeResource.h
	ConvolutionFilter.cpp
	Core.cpp
	DataController.cpp
	DataController.h
	DataControllerDefault.cpp
	DataControllerDefault.h
	DataExpression.cpp
	DataExpression.h
	DataModel.cpp
	DataModel.h
	DataModelHandle.cpp
	DataTypeRegister.cpp
	DataVariable.cpp
	DataView.cpp
	DataView.h
	DataViewDefault.cpp
	DataViewDefault.h
	Decorator.cpp
	DecoratorGradient.cpp
	DecoratorGradient.h
	DecoratorNinePatch.cpp
	DecoratorNinePatch.h
	DecoratorShader.cpp
	DecoratorShader.h
	DecoratorText.cpp
	DecoratorText.h
	DecoratorTiled.cpp
	DecoratorTiled.h
	DecoratorTiledBox.cpp
	DecoratorTiledBox.h
	DecoratorTiledHorizontal.cpp
	DecoratorTiledHorizontal.h
	DecoratorTiledImage.cpp
	DecoratorTiledImage.h
	DecoratorTiledVertical.cpp
	DecoratorTiledVertical.h
	DecoratorUtilities.cpp
	DecoratorUtilities.h
	DocumentHeader.cpp
	DocumentHeader.h
	EffectSpecification.cpp
	Element.cpp
	ElementAnimation.cpp
	ElementAnimation.h
	ElementBackgroundBorder.cpp
	ElementBackgroundBorder.h
	ElementDefinition.cpp
	ElementDefinition.h
	ElementDocument.cpp
	ElementEffects.cpp
	ElementEffects.h
	ElementHandle.cpp
	ElementHandle.h
	ElementInstancer.cpp
	ElementMeta.cpp
	ElementMeta.h
	ElementScroll.cpp
	ElementStyle.cpp
	ElementStyle.h
	ElementText.cpp
	ElementUtilities.cpp
	Event.cpp
	EventDispatcher.cpp
	EventDispatcher.h
	EventInstancer.cpp
	EventInstancerDefault.cpp
	EventInstancerDefault.h
	EventListenerInstancer.cpp
	EventSpecification.cpp
	EventSpecification.h
	Factory.cpp
	FileInterface.cpp
	FileInterfaceDefault.cpp
	FileInterfaceDefault.h
	Filter.cpp
	FilterBasic.cpp
	FilterBasic.h
	FilterBlur.cpp
	FilterBlur.h
	FilterDropShadow.cpp
	FilterDropShadow.h
	FontEffect.cpp
	FontEffectBlur.cpp
	FontEffectBlur.h
	FontEffectGlow.cpp
	FontEffectGlow.h
	FontEffectInstancer.cpp
	FontEffectOutline.cpp
	FontEffectOutline.h
	FontEffectShadow.cpp
	FontEffectShadow.h
	FontEngineInterface.cpp
	Geometry.cpp
	GeometryBackgroundBorder.cpp
	GeometryBackgroundBorder.h
	GeometryBoxShadow.cpp
	GeometryBoxShadow.h
	IdNameMap.h
	Log.cpp
	ListMarker.cpp
	ListMarker.h
	LogDefault.cpp
	LogDefault.h
	Math.cpp
	Memory.cpp
	Memory.h
	MeshUtilities.cpp
	ObserverPtr.cpp
	Plugin.cpp
	PluginRegistry.cpp
	PluginRegistry.h
	Pool.h
	precompiled.h
	Profiling.cpp
	PropertiesIterator.h
	PropertiesIteratorView.cpp
	Property.cpp
	PropertyDefinition.cpp
	PropertyDictionary.cpp
	PropertyParserAnimation.cpp
	PropertyParserAnimation.h
	PropertyParserBoxShadow.cpp
	PropertyParserBoxShadow.h
	PropertyParserColorStopList.cpp
	PropertyParserColorStopList.h
	PropertyParserColour.cpp
	PropertyParserColour.h
	PropertyParserDecorator.cpp
	PropertyParserDecorator.h
	PropertyParserFilter.cpp
	PropertyParserFilter.h
	PropertyParserFontEffect.cpp
	PropertyParserFontEffect.h
	PropertyParserKeyword.cpp
	PropertyParserKeyword.h
	PropertyParserNumber.cpp
	PropertyParserNumber.h
	PropertyParserRatio.cpp
	PropertyParserRatio.h
	PropertyParserString.cpp
	PropertyParserString.h
	PropertyParserTransform.cpp
	PropertyParserTransform.h
	PropertyShorthandDefinition.h
	PropertySpecification.cpp
	RenderInterface.cpp
	RenderInterfaceCompatibility.cpp
	RenderManager.cpp
	RenderManagerAccess.cpp
	RenderManagerAccess.h
	ScrollController.cpp
	ScrollController.h
	Spritesheet.cpp
	Stream.cpp
	StreamFile.cpp
	StreamFile.h
	StreamMemory.cpp
	StringUtilities.cpp
	StyleSheet.cpp
	StyleSheetContainer.cpp
	StyleSheetFactory.cpp
	StyleSheetFactory.h
	StyleSheetNode.cpp
	StyleSheetNode.h
	StyleSheetParser.cpp
	StyleSheetParser.h
	StyleSheetSelector.cpp
	StyleSheetSelector.h
	StyleSheetSpecification.cpp
	SystemInterface.cpp
	Template.cpp
	Template.h
	TemplateCache.cpp
	TemplateCache.h
	Texture.cpp
	TextureDatabase.cpp
	TextureDatabase.h
	TextureLayout.cpp
	TextureLayout.h
	TextureLayoutRectangle.cpp
	TextureLayoutRectangle.h
	TextureLayoutRow.cpp
	TextureLayoutRow.h
	TextureLayoutTexture.cpp
	TextureLayoutTexture.h
	Traits.cpp
	Transform.cpp
	TransformPrimitive.cpp
	TransformState.cpp
	TransformState.h
	TransformUtilities.cpp
	TransformUtilities.h
	Tween.cpp
	TypeConverter.cpp
	URL.cpp
	UserAgentStyleSheet.cpp
	UserAgentStyleSheet.h
	Variant.cpp
	WidgetScroll.cpp
	WidgetScroll.h
	XMLNodeHandler.cpp
	XMLNodeHandlerBody.cpp
	XMLNodeHandlerBody.h
	XMLNodeHandlerDefault.cpp
	XMLNodeHandlerDefault.h
	XMLNodeHandlerHead.cpp
	XMLNodeHandlerHead.h
	XMLNodeHandlerTemplate.cpp
	XMLNodeHandlerTemplate.h
	XMLParser.cpp
	XMLParseTools.cpp
	XMLParseTools.h
)

# Add public headers as files in the project (it's not necessary but convenient for IDE integration)
# Setting them as PRIVATE so that it's addition doesn't propagate, it won't affect availability since
# the entire include directory has already been declared as public
target_sources(rmlui_core PRIVATE
	"${PP_UI_INCLUDE_DIR}/RmlUi/Config/Config.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/BaseXMLParser.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Box.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/CallbackTexture.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Colour.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Colour.inl"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/CompiledFilterShader.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/ComputedValues.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Containers/itlib/flat_map.hpp"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Containers/itlib/flat_set.hpp"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Containers/robin_hood.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Context.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/ContextInstancer.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/ConvolutionFilter.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Core.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/DataModelHandle.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/DataStructHandle.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/DataTypeRegister.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/DataTypes.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/DataVariable.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Debug.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/DecorationTypes.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Decorator.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Dictionary.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/EffectSpecification.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Element.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Element.inl"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/ElementDocument.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/ElementInstancer.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/ElementScroll.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Elements/ElementForm.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Elements/ElementFormControl.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Elements/ElementFormControlInput.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Elements/ElementFormControlSelect.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Elements/ElementFormControlTextArea.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Elements/ElementProgress.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Elements/ElementTabSet.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/ElementText.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/ElementUtilities.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Event.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/EventInstancer.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/EventListener.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/EventListenerInstancer.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Factory.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/FileInterface.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Filter.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/FontEffect.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/FontEffectInstancer.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/FontEngineInterface.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/FontGlyph.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/FontMetrics.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Geometry.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Header.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/ID.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Input.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Log.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Math.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Matrix4.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Matrix4.inl"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Mesh.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/MeshUtilities.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/NumericValue.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/ObserverPtr.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Platform.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Plugin.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Profiling.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/PropertiesIteratorView.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Property.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/PropertyDefinition.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/PropertyDictionary.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/PropertyIdSet.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/PropertyParser.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/PropertySpecification.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Rectangle.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/RenderBox.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/RenderInterface.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/RenderInterfaceCompatibility.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/RenderManager.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/ScriptInterface.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/ScrollTypes.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/SelectionTypes.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/TextLoupe.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Span.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Spritesheet.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/StableVector.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Stream.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/StreamMemory.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/StringUtilities.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/StyleSheet.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/StyleSheetContainer.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/StyleSheetSpecification.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/StyleSheetTypes.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/StyleTypes.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/SystemInterface.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/TextInputContext.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/TextInputHandler.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/TextShapingContext.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Texture.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Traits.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Transform.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/TransformPrimitive.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Tween.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/TypeConverter.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/TypeConverter.inl"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Types.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/UniqueRenderResource.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Unit.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/URL.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Utilities.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Variant.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Variant.inl"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Vector2.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Vector2.inl"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Vector3.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Vector3.inl"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Vector4.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Vector4.inl"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/Vertex.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/XMLNodeHandler.h"
	"${PP_UI_INCLUDE_DIR}/RmlUi/Core/XMLParser.h"
)

set_common_target_options(rmlui_core)

target_include_directories(rmlui_core PRIVATE "${PP_UI_INCLUDE_DIR}")
target_include_directories(rmlui_core INTERFACE
	"$<BUILD_INTERFACE:${PP_UI_INCLUDE_DIR}>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

add_library(RmlUi::Core ALIAS rmlui_core)

set_target_properties(rmlui_core PROPERTIES
	# Add export name so that it can be exported with a namespaced name instead
	# of using the name we actually used to declare the target
	EXPORT_NAME "Core"

	# Change output name of the final library file
	OUTPUT_NAME "rmlui"
)

set(RMLUI_VERSION_SHORT "6.2")
target_compile_definitions(rmlui_core PRIVATE "RMLUI_VERSION=\"${RMLUI_VERSION_SHORT}\"")

option(RMLUI_DEBUG_SELECTION_HANDLES "Draw magenta/yellow debug markers at selection handle anchors" OFF)
if(RMLUI_DEBUG_SELECTION_HANDLES)
	target_compile_definitions(rmlui_core PRIVATE RMLUI_DEBUG_SELECTION_HANDLES)
endif()

add_subdirectory("Elements")
add_subdirectory("Layout")

# Set up definitions to export functions and classes as appropriate
get_target_property(rmlui_core_TYPE rmlui_core "TYPE")
if(rmlui_core_TYPE STREQUAL "STATIC_LIBRARY")
	# If RmlUi is being compiled as a static library, notify code to disable usage of __dllspec()
	# Since public headers also detect it, it needs to be a public definition
	target_compile_definitions(rmlui_core PUBLIC "RMLUI_STATIC_LIB")
elseif(rmlui_core_TYPE STREQUAL "SHARED_LIBRARY")
	# If RmlUi is being compiled as a shared library, notify code to export functions using __dllspec()
	# For applications consuming the library, the headers will automatically use dllimport
	# Platform and compiler handling is already done by code via pre-processor macros
	target_compile_definitions(rmlui_core PRIVATE "RMLUI_CORE_EXPORTS")
endif()
unset(rmlui_core_TYPE)

if(RMLUI_FONT_ENGINE STREQUAL "freetype")
	# Include the source files for the default font engine.
	add_subdirectory("FontEngineDefault")

	if(RMLUI_FONT_ENGINE_HARFBUZZ)
		add_subdirectory("FontEngineHarfBuzz")
		target_link_libraries(rmlui_core PRIVATE harfbuzz::harfbuzz)
	endif()

	# RMLUI_CMAKE_MINIMUM_VERSION_RAISE_NOTICE:
	# From CMake 3.13 the next line can be moved into `FontEngineDefault/CMakeLists.txt`, see CMP0079.
	target_link_libraries(rmlui_core PRIVATE Freetype::Freetype)
endif()

if(RMLUI_LOTTIE_PLUGIN)
	# RMLUI_CMAKE_MINIMUM_VERSION_RAISE_NOTICE:
	# From CMake 3.13 we could move this to `Lottie/CMakeLists.txt`, see CMP0079.
	target_link_libraries(rmlui_core PRIVATE rlottie::rlottie)
endif()

if(RMLUI_SVG_PLUGIN)
	# RMLUI_CMAKE_MINIMUM_VERSION_RAISE_NOTICE:
	# From CMake 3.13 we could move this to `Lottie/CMakeLists.txt`, see CMP0079.
	target_link_libraries(rmlui_core PRIVATE lunasvg::lunasvg)
endif()

if(RMLUI_TRACY_PROFILING)
	if(CMAKE_CONFIGURATION_TYPES AND RMLUI_TRACY_CONFIGURATION)
		target_link_libraries(rmlui_core PUBLIC "$<$<CONFIG:Tracy>:Tracy::TracyClient>")
		target_compile_definitions(rmlui_core PUBLIC "$<$<CONFIG:Tracy>:RMLUI_TRACY_PROFILING>")
		if(RMLUI_TRACY_MEMORY_PROFILING)
			target_compile_definitions(rmlui_core PRIVATE "$<$<CONFIG:Tracy>:RMLUI_TRACY_MEMORY_PROFILING>")
		endif()
		message(STATUS "Tracy profiling enabled in configuration `Tracy`.")
	else()
		target_link_libraries(rmlui_core PUBLIC Tracy::TracyClient)
		target_compile_definitions(rmlui_core PUBLIC "RMLUI_TRACY_PROFILING")
		if(RMLUI_TRACY_MEMORY_PROFILING)
			target_compile_definitions(rmlui_core PRIVATE "RMLUI_TRACY_MEMORY_PROFILING")
		endif()
		message(STATUS "Tracy profiling enabled.")
	endif()
endif()

if(NOT RMLUI_THIRDPARTY_CONTAINERS)
	target_compile_definitions(rmlui_core PUBLIC "RMLUI_NO_THIRDPARTY_CONTAINERS")
	message(STATUS "Disabling third-party containers for RmlUi.")
endif()

if(RMLUI_CUSTOM_RTTI)
	target_compile_definitions(rmlui_core PUBLIC "RMLUI_CUSTOM_RTTI")
	message(STATUS "Enabling custom RTTI for RmlUi.")
endif()

if(RMLUI_MATRIX_ROW_MAJOR)
	target_compile_definitions(rmlui_core PUBLIC "RMLUI_MATRIX_ROW_MAJOR")
	message(STATUS "Configuring RmlUi to use row-major matrix types.")
endif()

# RMLUI_CMAKE_MINIMUM_VERSION_RAISE_NOTICE:
# From CMake 3.16 we can skip the version check.
if(RMLUI_PRECOMPILED_HEADERS AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
	target_precompile_headers(rmlui_core PRIVATE "${CMAKE_SOURCE_DIR}/src/core/precompiled.h")
elseif(RMLUI_PRECOMPILED_HEADERS)
	message(STATUS "Could not enable precompiled headers, requires CMake version 3.16 or greater.")
endif()

if(RMLUI_CUSTOM_CONFIGURATION AND RMLUI_CUSTOM_CONFIGURATION_FILE)
	target_compile_definitions(rmlui_core PUBLIC "RMLUI_CUSTOM_CONFIGURATION_FILE=\"${RMLUI_CUSTOM_CONFIGURATION_FILE}\"")
	message(STATUS "Including ${RMLUI_CUSTOM_CONFIGURATION_FILE} instead of <RmlUi/Config/Config.h>")
endif()
if(RMLUI_CUSTOM_CONFIGURATION AND RMLUI_CUSTOM_INCLUDE_DIRS)
	target_include_directories(rmlui_core PUBLIC ${RMLUI_CUSTOM_INCLUDE_DIRS})
endif()
if(RMLUI_CUSTOM_CONFIGURATION AND RMLUI_CUSTOM_LINK_LIBRARIES)
	target_link_libraries(rmlui_core PUBLIC ${RMLUI_CUSTOM_LINK_LIBRARIES})
endif()

# RMLUI_CMAKE_MINIMUM_VERSION_RAISE_NOTICE:
# We use default paths provided from GNUInstallDirs. From CMake 3.14 these paths (CMAKE_INSTALL_...) will be used
# automatically and can be removed from the following call. The same applies to many other calls to install(TARGETS...).
# Note that GNUInstallDirs should still be included in the project root.
install(TARGETS rmlui_core
	EXPORT RmlUiTargets
	${RMLUI_RUNTIME_DEPENDENCY_SET_ARG}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install_target_pdb(rmlui_core)
