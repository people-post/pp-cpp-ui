#include <ui/platform/TextInputProperties_SDL.h>
#include <doctest.h>

TEST_SUITE_BEGIN("PasswordKeyboard");

TEST_CASE("a password field asks SDL for secure entry without autocorrect or capitals") {
  const SDL_PropertiesID props = SDL_CreateProperties();
  REQUIRE(props);
  RmlSDL::SetTextInputProperties(props, ui::KeyboardType::Password);
  CHECK(SDL_GetNumberProperty(props, SDL_PROP_TEXTINPUT_TYPE_NUMBER, -1) == SDL_TEXTINPUT_TYPE_TEXT_PASSWORD_HIDDEN);
  CHECK(SDL_GetNumberProperty(props, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER, -1) == SDL_CAPITALIZE_NONE);
  CHECK(SDL_GetBooleanProperty(props, SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN, true) == false);
  SDL_DestroyProperties(props);
}

TEST_CASE("a text field keeps SDL's default keyboard") {
  const SDL_PropertiesID props = SDL_CreateProperties();
  REQUIRE(props);
  RmlSDL::SetTextInputProperties(props, ui::KeyboardType::Text);
  CHECK_FALSE(SDL_HasProperty(props, SDL_PROP_TEXTINPUT_TYPE_NUMBER));
  CHECK_FALSE(SDL_HasProperty(props, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER));
  CHECK_FALSE(SDL_HasProperty(props, SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN));
  SDL_DestroyProperties(props);
}

TEST_SUITE_END();
