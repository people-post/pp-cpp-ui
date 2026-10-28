# Agent guide

`pp-cpp-ui` owns the UI engine and SDL/OpenGL backends consumed by pp-browser.
Read `docs/SRC_LAYOUT.md` and `docs/PROVENANCE.md` for module boundaries and upstream history.

- Public UI headers: `include/ui/`; implementation: `src/`.
- Platform/render integration: `src/platform/`, `src/render/`; vendored SDL camera drivers:
  `third_party/sdl3/src/camera/`.
- Module tests: `src/*/tests/`, registered in each module's CMake file; shared harness: `tests/`.
- Keep vendored changes focused and document their reason and upstream source.
- Native camera configuration must keep the requested format and frame duration consistent;
  compiling the driver does not establish actual device FPS or first-frame latency.
- Local pp-browser builds may consume this checkout directly; preserve unrelated edits and
  report any local dependency change that still needs a released dependency pin.

Build/tests: `cmake -S . -B build -DPP_UI_BUILD_TESTS=ON`, then build `pp_ui_backend`
and `ui_unit_tests`; use device consumer builds for native camera validation.
