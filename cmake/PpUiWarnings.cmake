# Warning flags for first-party pp-cpp-ui targets (the engine modules, the SDL / GL3 backend, the
# tests), as pp-browser sets them (its cmake/PpBrowserWarnings.cmake). Does not apply to
# third_party/ or the vendored test dependencies, which keep their own options.

option(PP_UI_WARNINGS_AS_ERRORS
  "Treat compiler warnings as errors for first-party pp-cpp-ui targets" ON)

function(pp_ui_apply_warnings target)
  if(NOT TARGET ${target})
    message(FATAL_ERROR "pp_ui_apply_warnings: target '${target}' does not exist")
  endif()

  get_target_property(_pp_warn_type ${target} TYPE)
  if(_pp_warn_type STREQUAL "INTERFACE_LIBRARY")
    return()
  endif()

  if(MSVC)
    # As GCC / Clang below under /WX:
    # - CRT secure deprecations (getenv / localtime) — portable code uses the ISO APIs
    # - C4100 unused parameter — same as -Wno-unused-parameter
    # - C4458 hide class member — GCC does not enable -Wshadow by default
    target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(${target} PRIVATE /W4 /wd4100 /wd4458)
    if(PP_UI_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE /WX)
    endif()
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU|AppleClang")
    # From -Wextra, suppress the two that are chronically noisy:
    # - missing-field-initializers: aggregate / designated init omitting fields
    # - unused-parameter: interface overrides and callback stubs
    target_compile_options(${target} PRIVATE
      -Wall
      -Wextra
      -Wno-missing-field-initializers
      -Wno-unused-parameter)
    # GCC 13's -Wdangling-reference flags every reference bound through a part view returned by
    # value (`const Box& b = element->BoxModel().GetBox();` — ElementBox, ElementStyle… point into
    # the element, which outlives them). GCC 14 no longer warns for such reference-like classes.
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13
       AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
      target_compile_options(${target} PRIVATE -Wno-dangling-reference)
    endif()
    if(PP_UI_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
  endif()
endfunction()
