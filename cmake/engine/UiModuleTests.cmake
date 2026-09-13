#[[
  ui_add_module_tests(<module> <source.cpp>...)

  Registers unit-test sources owned by src/<module>/tests/ into the shared
  ui_unit_tests executable (assembled later under tests/engine).
  Call only from a module's tests/CMakeLists.txt when UI_TESTS is on.
]]

function(ui_add_module_tests module)
	if(NOT ARGN)
		message(FATAL_ERROR "ui_add_module_tests(${module}): no sources given")
	endif()
	foreach(src IN LISTS ARGN)
		if(IS_ABSOLUTE "${src}")
			set(_path "${src}")
		else()
			set(_path "${CMAKE_CURRENT_SOURCE_DIR}/${src}")
		endif()
		if(NOT EXISTS "${_path}")
			message(FATAL_ERROR "ui_add_module_tests(${module}): missing ${_path}")
		endif()
		set_property(GLOBAL APPEND PROPERTY UI_UNIT_TEST_SOURCES "${_path}")
	endforeach()
endfunction()
