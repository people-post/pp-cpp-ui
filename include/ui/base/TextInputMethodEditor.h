#pragma once

#include <ui/base/StringUtilities.h>
#include <ui/base/TextInputHandler.h>

namespace ui {

class TextInputContext;

/**
    A text input handler that shows an IME composition (marked text, preedit) in the focused text field.

    A platform layer feeds it the composition the IME reports and the text the IME finally commits. The composition is
    written into the field at the caret (replacing any selection) and underlined by the widget; Commit() turns it into
    ordinary text, an empty composition or a loss of focus takes it out again.

    All offsets are in characters.
 */
class UI_CORE_API TextInputMethodEditor : public TextInputHandler {
public:
  void OnActivate(TextInputContext* input_context) override;
  void OnDeactivate(TextInputContext* input_context) override;
  void OnDestroy(TextInputContext* input_context) override;

  /// Shows the text being composed, replacing the previous composition. An empty text removes the composition.
  /// @param[in] text The composition.
  /// @param[in] caret The caret position within the composition, or a negative value for its end.
  void SetComposition(StringView text, int caret);
  /// Replaces the composition with the text the IME committed.
  /// @return False if nothing is being composed; the caller then inserts the text as ordinary typing.
  bool Commit(StringView text);
  /// Removes the composition from the field.
  void Cancel();

  /// True while a composition is shown in the active field. Key presses then belong to the IME.
  /// A composition the app wiped out by replacing the value counts as ended.
  bool IsComposing();

private:
  void EndComposition();

  TextInputContext* input_context = nullptr;
  bool composing = false;
  int range_start = 0;
  int range_end = 0;
};

} // namespace ui
