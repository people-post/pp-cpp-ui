add_library(ui_debugger
	BeaconSource.h
	CommonSource.h
	Debugger.cpp
	DebuggerPlugin.cpp
	DebuggerPlugin.h
	DebuggerSystemInterface.cpp
	DebuggerSystemInterface.h
	ElementContextHook.cpp
	ElementContextHook.h
	ElementDataModels.cpp
	ElementDataModels.h
	ElementDebugDocument.cpp
	ElementDebugDocument.h
	ElementInfo.cpp
	ElementInfo.h
	ElementLog.cpp
	ElementLog.h
	FontSource.h
	Geometry.cpp
	Geometry.h
	InfoSource.h
	LogSource.h
	MenuSource.h
)

set_common_target_options(ui_debugger)

target_include_directories(ui_debugger PRIVATE "${PP_UI_INCLUDE_DIR}" "${CMAKE_SOURCE_DIR}/src")
target_include_directories(ui_debugger INTERFACE
	"$<BUILD_INTERFACE:${PP_UI_INCLUDE_DIR}>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Add public headers as files in the project (it's not necessary but convenient for IDE integration)
# Setting them as PRIVATE so that it's addition doesn't propagate, it won't affect availability since
# the entire include directory has already been declared as public
target_sources(ui_debugger PRIVATE
	"${PP_UI_INCLUDE_DIR}/ui/Debugger/Debugger.h"
	"${PP_UI_INCLUDE_DIR}/ui/Debugger/Header.h"
	"${PP_UI_INCLUDE_DIR}/ui/Debugger.h"
)

target_link_libraries(ui_debugger PUBLIC ui_core)

add_library(ui::debugger ALIAS ui_debugger)

# Set up definitions to export functions and classes as appropriate
get_target_property(ui_debugger_TYPE ui_debugger "TYPE")
if(ui_debugger_TYPE STREQUAL "SHARED_LIBRARY")
	# If pp-cpp-ui debugger is being compiled as a shared library, notify code to export functions using __dllspec()
	# For applications consuming the library, the headers will automatically use dllimport
	# Platform and compiler handling is already done by code via pre-processor macros
	target_compile_definitions(ui_debugger PRIVATE "UI_DEBUGGER_EXPORTS")
endif()

# Set additional target properties
set_target_properties(ui_debugger PROPERTIES
	# Add export name so that it can be exported with a namespaced name instead
	# of using the name we actually used to declare the target
	EXPORT_NAME "Debugger"
)

install(TARGETS ui_debugger
	EXPORT UiTargets
	${UI_RUNTIME_DEPENDENCY_SET_ARG}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install_target_pdb(ui_debugger)
