#pragma once

#include <ui/dom/Input.h>
#include <ui/paint/RenderInterface.h>
#include <ui/core/SystemInterface.h>
#include <ui/base/Types.h>
#ifndef UI_SDL_VERSION_MAJOR
#define UI_SDL_VERSION_MAJOR 2
#endif

#if UI_SDL_VERSION_MAJOR >= 3
#include <SDL3/SDL.h>
#endif

using KeyDownCallback = bool (*)(ui::Context* context, ui::Input::KeyIdentifier key, int key_modifier, float native_dp_ratio, bool priority);
#if UI_SDL_VERSION_MAJOR >= 3
using PreProcessEventCallback = bool (*)(ui::Context* context, SDL_Event& event, bool& propagate_event);
#endif

namespace Backend {

bool Initialize(const char* window_name, int width, int height, bool allow_resize);
void Shutdown();

ui::SystemInterface* GetSystemInterface();
ui::RenderInterface* GetRenderInterface();

void SyncContext(ui::Context* context);

#if UI_SDL_VERSION_MAJOR >= 3
void SetPreProcessEventHandler(PreProcessEventCallback callback);
SDL_Window* GetWindow();
#endif

bool ProcessEvents(ui::Context* context, KeyDownCallback key_down_callback = nullptr, bool power_save = false);
void RequestExit();

void BeginFrame();
void PresentFrame();

} // namespace Backend
