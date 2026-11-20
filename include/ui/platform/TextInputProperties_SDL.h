#pragma once

#include <ui/base/SystemInterface.h>
#include <SDL3/SDL.h>

namespace RmlSDL {

// Fills the SDL text input properties for the keyboard a field asks for. A password field gets the platform's secure
// entry (iOS secureTextEntry, Android's password input type) with autocorrect and capitalization off, so the keyboard
// neither suggests nor learns what is typed. A text field sets nothing and keeps SDL's defaults.
inline void SetTextInputProperties(SDL_PropertiesID props, ui::KeyboardType type) {
  if (type != ui::KeyboardType::Password)
    return;
  SDL_SetNumberProperty(props, SDL_PROP_TEXTINPUT_TYPE_NUMBER, SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_HIDDEN);
  SDL_SetNumberProperty(props, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER, SDL_CAPITALIZE_NONE);
  SDL_SetBooleanProperty(props, SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN, false);
}

} // namespace RmlSDL
