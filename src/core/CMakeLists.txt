# Thin bootstrap module + ui_core target settings.
# Sources for other modules are attached via their own CMakeLists (target_sources).

target_sources(ui_core PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/Core.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FileInterface.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FileInterfaceDefault.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/FileInterfaceDefault.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/Plugin.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PluginRegistry.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/PluginRegistry.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/SystemInterface.cpp"
)

# Single private root: use #include "module/Header.h" for cross-module private headers.
# Same-folder #include "Header.h" still works via the including file's directory.
target_include_directories(ui_core PRIVATE
	"${CMAKE_SOURCE_DIR}/src"
)

set_common_target_options(ui_core)

target_include_directories(ui_core PRIVATE "${PP_UI_INCLUDE_DIR}")
target_include_directories(ui_core INTERFACE
	"$<BUILD_INTERFACE:${PP_UI_INCLUDE_DIR}>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

add_library(ui::core ALIAS ui_core)

set_target_properties(ui_core PROPERTIES
	# Add export name so that it can be exported with a namespaced name instead
	# of using the name we actually used to declare the target
	EXPORT_NAME "Core"

	# Change output name of the final library file
	OUTPUT_NAME "ui"
)

set(UI_VERSION_SHORT "6.2")
target_compile_definitions(ui_core PRIVATE "UI_VERSION=\"${UI_VERSION_SHORT}\"")

option(UI_DEBUG_SELECTION_HANDLES "Draw magenta/yellow debug markers at selection handle anchors" OFF)
if(UI_DEBUG_SELECTION_HANDLES)
	target_compile_definitions(ui_core PRIVATE UI_DEBUG_SELECTION_HANDLES)
endif()


# Set up definitions to export functions and classes as appropriate
get_target_property(ui_core_TYPE ui_core "TYPE")
if(ui_core_TYPE STREQUAL "STATIC_LIBRARY")
	# If pp-cpp-ui is being compiled as a static library, notify code to disable usage of __dllspec()
	# Since public headers also detect it, it needs to be a public definition
	target_compile_definitions(ui_core PUBLIC "UI_STATIC_LIB")
elseif(ui_core_TYPE STREQUAL "SHARED_LIBRARY")
	# If pp-cpp-ui is being compiled as a shared library, notify code to export functions using __dllspec()
	# For applications consuming the library, the headers will automatically use dllimport
	# Platform and compiler handling is already done by code via pre-processor macros
	target_compile_definitions(ui_core PRIVATE "UI_CORE_EXPORTS")
endif()
unset(ui_core_TYPE)


if(UI_LOTTIE_PLUGIN)
	# UI_CMAKE_MINIMUM_VERSION_RAISE_NOTICE:
	# From CMake 3.13 we could move this to `Lottie/CMakeLists.txt`, see CMP0079.
	target_link_libraries(ui_core PRIVATE rlottie::rlottie)
endif()

if(UI_SVG_PLUGIN)
	# UI_CMAKE_MINIMUM_VERSION_RAISE_NOTICE:
	# From CMake 3.13 we could move this to `Lottie/CMakeLists.txt`, see CMP0079.
	target_link_libraries(ui_core PRIVATE lunasvg::lunasvg)
endif()

if(UI_TRACY_PROFILING)
	if(CMAKE_CONFIGURATION_TYPES AND UI_TRACY_CONFIGURATION)
		target_link_libraries(ui_core PUBLIC "$<$<CONFIG:Tracy>:Tracy::TracyClient>")
		target_compile_definitions(ui_core PUBLIC "$<$<CONFIG:Tracy>:UI_TRACY_PROFILING>")
		if(UI_TRACY_MEMORY_PROFILING)
			target_compile_definitions(ui_core PRIVATE "$<$<CONFIG:Tracy>:UI_TRACY_MEMORY_PROFILING>")
		endif()
		message(STATUS "Tracy profiling enabled in configuration `Tracy`.")
	else()
		target_link_libraries(ui_core PUBLIC Tracy::TracyClient)
		target_compile_definitions(ui_core PUBLIC "UI_TRACY_PROFILING")
		if(UI_TRACY_MEMORY_PROFILING)
			target_compile_definitions(ui_core PRIVATE "UI_TRACY_MEMORY_PROFILING")
		endif()
		message(STATUS "Tracy profiling enabled.")
	endif()
endif()

if(NOT UI_THIRDPARTY_CONTAINERS)
	target_compile_definitions(ui_core PUBLIC "UI_NO_THIRDPARTY_CONTAINERS")
	message(STATUS "Disabling third-party containers for pp-cpp-ui.")
endif()

if(UI_CUSTOM_RTTI)
	target_compile_definitions(ui_core PUBLIC "UI_CUSTOM_RTTI")
	message(STATUS "Enabling custom RTTI for pp-cpp-ui.")
endif()

if(UI_MATRIX_ROW_MAJOR)
	target_compile_definitions(ui_core PUBLIC "UI_MATRIX_ROW_MAJOR")
	message(STATUS "Configuring pp-cpp-ui to use row-major matrix types.")
endif()

# UI_CMAKE_MINIMUM_VERSION_RAISE_NOTICE:
# From CMake 3.16 we can skip the version check.
if(UI_PRECOMPILED_HEADERS AND CMAKE_VERSION VERSION_GREATER_EQUAL "3.16")
	target_precompile_headers(ui_core PRIVATE "${CMAKE_SOURCE_DIR}/src/base/precompiled.h")
elseif(UI_PRECOMPILED_HEADERS)
	message(STATUS "Could not enable precompiled headers, requires CMake version 3.16 or greater.")
endif()

if(UI_CUSTOM_CONFIGURATION AND UI_CUSTOM_CONFIGURATION_FILE)
	target_compile_definitions(ui_core PUBLIC "UI_CUSTOM_CONFIGURATION_FILE=\"${UI_CUSTOM_CONFIGURATION_FILE}\"")
	message(STATUS "Including ${UI_CUSTOM_CONFIGURATION_FILE} instead of <ui/config/Config.h>")
endif()
if(UI_CUSTOM_CONFIGURATION AND UI_CUSTOM_INCLUDE_DIRS)
	target_include_directories(ui_core PUBLIC ${UI_CUSTOM_INCLUDE_DIRS})
endif()
if(UI_CUSTOM_CONFIGURATION AND UI_CUSTOM_LINK_LIBRARIES)
	target_link_libraries(ui_core PUBLIC ${UI_CUSTOM_LINK_LIBRARIES})
endif()

# UI_CMAKE_MINIMUM_VERSION_RAISE_NOTICE:
# We use default paths provided from GNUInstallDirs. From CMake 3.14 these paths (CMAKE_INSTALL_...) will be used
# automatically and can be removed from the following call. The same applies to many other calls to install(TARGETS...).
# Note that GNUInstallDirs should still be included in the project root.
install(TARGETS ui_core
	EXPORT UiTargets
	${UI_RUNTIME_DEPENDENCY_SET_ARG}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install_target_pdb(ui_core)

