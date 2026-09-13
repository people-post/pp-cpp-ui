#pragma once

#include <ui/base/Header.h>
#include <ui/base/Types.h>
#include <ui/base/Vector2.h>
#include <functional>

namespace ui {

class RenderManager;

struct TextLoupeState {
	bool active = false;
	Vector2f anchor;
};

enum class TextLoupePhase { Capture, Draw };

using TextLoupeRenderCallback = std::function<void(TextLoupePhase phase, const TextLoupeState& state, RenderManager& render_manager)>;

} // namespace ui
